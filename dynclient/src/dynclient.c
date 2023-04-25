#include "echo_mod_dyn_common.h"
#include <echo_mod_dyn/IDynPing.idl.h>

/*
 * В этой процедуре клиент получает из сервера имен список доступных
 * серверов и их служб. Процедура перебирает индексы, пока не 
 * получит ошибку.
 * Регистрация сервера и заспрос клиента - асинхронные события,
 * поэтому перечисление доступных сервисов выполняется с таймаутом.
 */

static Retcode GetService(
                NsHandle ns, char *server, char *service, rtl_size_t timeout)
{
    unsigned int index=0;
    Retcode rc = rcOk;
    rtl_size_t start = KnGetMSecSinceStart();

    do
    {
        rc = NsEnumServices(
                        ns,
                        InterfaceType,
                        index,
                        server,
                        UCORE_STRING_SIZE,
                        service,
                        UCORE_STRING_SIZE);

        if (rc == rcResourceNotFound)
        {
            rtl_size_t delta = KnGetMSecSinceStart() - start;

            if (delta < timeout)
            {
                KnSleep((rtl_uint32_t) rtl_min(RISEUP_TIMEOUT_MSEC,
                                               (timeout - delta)));
                fprintf(stderr, ".");                                                
            }
            else
            {
                rc = rcTimeout;
                fprintf(stderr, "\n[DynClient] NsEnum timeout\n");                
            }
        }
        else 
        {
            fprintf(stderr, "[DynClient] NsEnum found: index %i, server %s, service %s\n", index, server, service); 
            index++;
        }
    }
    while (rc != rcTimeout);

    if (index>0)
        return rcOk;
    else   
        return rcResourceNotFound;
}

/*
 * В этой процедуре выполняется подключение к серверу имен и вызов процедуры
 * перечисления доступных серверов и сервисов.
 */


static Retcode ConnectToServer(char *server, char *service)
{
    Retcode rc = rcOk;
    NsHandle ns = NS_INVALID_HANDLE;

    rc = NsCreate(RTL_NULL, NS_WAIT_TIME_MS, &ns);

    if (rc == rcOk)
    {
        rc = GetService(ns, server, service, TIMEOUT_MSEC);
    }

    return rc;
}

/* Точка запуска клиента. */
int main(void)
{
    Retcode rc = rcOk;
    Handle handle = INVALID_HANDLE;
    rtl_uint32_t rsid = INVALID_HANDLE;
    struct IDynPing_proxy proxy;
    NkKosTransport transport;
    char server[UCORE_STRING_SIZE];
    char service[UCORE_STRING_SIZE];
    int i=0;

    fprintf(stderr, "[DynClient] Starting!\n");

    /* Для динамического соединения необходимо сначала подключиться к
     * сервису NameServer и с его помощью найти хендл необходимого 
     * интерфейса. Нобходимо не забыть выдать соответствующие разрешения
     * в psl-файле.
     */
    rc = ConnectToServer(server, service);
    if (rc != rcOk)
    {
        fprintf(stderr, "[DynClient]: Can not establish connection!\n");
        return EXIT_FAILURE;
    }

    /*
     * Здесь клиент получает хендл рабочего сервера от сервера имен.
     * Если соединение установить не удается - клиент останавливается.
     */
    rc = KnCmConnect(server, service, TIMEOUT_MSEC, &handle, &rsid);
    if(rc != rcOk)
    {
        fprintf(stderr, "[DynClient]: Can not get Server descriptor!\n");
        return EXIT_FAILURE;
    }

    /* Далее инициализация идентична варианту со статическим соединением. */
    NkKosTransport_Init(&transport, handle, NK_NULL, 0);

    assert(NK_IID_MAX >= rsid);
    IDynPing_proxy_init(&proxy, &transport.base, (nk_iid_t) rsid);

    /* Используем только константные части, вместо арен используем NULL. */
    IDynPing_FDynPing_req req;
    IDynPing_FDynPing_res res;

    req.value = EXAMPLE_VALUE_TO_SEND;

    for (i = 0; i < 10; ++i)
    {
        fprintf(stderr, "[DynClient] sending %d\n", (int) req.value);
        /* Вызываем функцию интерфейса. */
        if (IDynPing_FDynPing(&proxy.base, &req, NULL, &res, NULL) == rcOk)
        {
            fprintf(stderr, "[DynClient] received result %d\n", (int) res.result);
            req.value = res.result;
        }
        else
            fprintf(stderr, "[DynClient]: Failed to call IDynPing_FDynPing!\n");
    }

    fprintf(stderr, "[DynClient]: Finished, exiting\n");
    return EXIT_SUCCESS;
}
