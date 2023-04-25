#include "echo_mod_dyn_common.h"

#include <stdbool.h>
#include <rtl/stdio.h>
#include <rtl/string.h>

#include <echo_mod_dyn/DynServer.edl.h>

/* Заготовим константы, как будут представляться сервер и интерфейс. */

static const char DynServerName[] =    "echo_mod_dyn.DynServer";
static const char InterfaceName[] =    "pinginst";


/* Реализация метода почти идентична echo_mod. */
static nk_err_t FDynPing_impl(
                        __rtl_unused struct IDynPing                *self,
                        __rtl_unused const IDynPing_FDynPing_req    *req,
                        __rtl_unused const struct nk_arena          *reqArena,
                        IDynPing_FDynPing_res                       *res,
                        __rtl_unused struct nk_arena                *resArena)
{
    res->result = req->value + 3;
    fprintf(stderr, "[DynServer] request %d, result %d\n", (int) req->value, (int) res->result);
    return NK_EOK;
}

/* Конструктор интерфейса также идентичен. */
static struct IDynPing *CreateIDynPingImpl(void)
{
    static const struct IDynPing_ops ops =
    {
        .FDynPing = FDynPing_impl
    };

    static struct IDynPing obj =
    {
        .ops = &ops
    };

    return &obj;
}

/**
 * Это процедура, которая выполняет подключение клиентов к серверу.
 * В этом примере она упрощена до подключения только одного клиента.
 * Пример подключения нескольких клиентов можно посмотреть в defer_to_kernel.
 */
static Retcode ConnectWithClients(Handle *endpoint)
{
    Retcode rc = rcFail;
    char clientName[UCORE_STRING_SIZE];
    char serviceName[UCORE_STRING_SIZE];
    Handle listenerHandle = INVALID_HANDLE;

    /* Это собственно проверка входящих запросов от клиентов. Есть
     * таймаут ожидания, но выход из функции выполняется при первом
     * найденном клиенте в очереди. Пока не будет выполнен KnCmAccept
     * или KnCmDrop - KnCmListen будет возвращать одно и то же.
    */
    rc = KnCmListen(
            DynServerName,
            INFINITE_TIMEOUT,
            clientName,
            serviceName);
    if (rc == rcOk)
    {
        Handle serverHandle;
        fprintf(stderr, "[DynServer] Accepting connect from %s to %s\n", clientName, serviceName);
        rc = KnCmAccept(
                clientName,
                serviceName,
                DynServer_pinginst_iid, // iid генерируется автоматически при обработке xDL
                listenerHandle,
                &serverHandle);
        if ((rc == rcOk) && (listenerHandle == INVALID_HANDLE))
        {
            listenerHandle = serverHandle;
            /* listenerHandle - это промежуточный кусок соединения, ведущий к входящему клиенту.
             * Ему надо присвоить хендл, ведущий на сервер. Если он равен INVALID_HANDLE -
             * значит, это выполняется Accept первого клиента.
             */
        }
    }

    if (rc == rcOk)
    {
       *endpoint = listenerHandle;
       /* endpoint - это выходной параметр текущей процедуры, через него надо отдать
        * слушающий хендл (фактически - хендл сервера) клиенту для инициализации
        * транспортной части.
        */
    }
    return rc;
}

/* Точка входа в сервер. */
int main(void)
{
    Retcode rc = rcOk;
    NsHandle ns = NS_INVALID_HANDLE;
    NkKosTransport transport;

    fprintf(stderr, "[DynServer] starting!\n");

    /**
     * Начинаем с регистрации рабочего сервера на сервере имен.
     * Для этого рабочий сервер и сервер имен должны иметь необходимые права.
     */
    rc = NsCreate(RTL_NULL, NS_WAIT_TIME_MS, &ns);
    if (rc != rcOk)
    {
        fprintf(stderr, "[DynServer]: Can't connect to Name Server, terminating.\n");
        return EXIT_FAILURE;
    }

    /**
     * После открытия соединения необходимо сообщить серверу имен все доступные
     * интерфейсы данного сервера. Интерфейсы перечисляются экземплярами, а не типами.
     */
    rc = NsPublishService(ns, InterfaceType, DynServerName, InterfaceName);
    if (rc != rcOk)
    {
        fprintf(stderr, "[DynServer]: Can't publish service to Name Server, terminating.\n");
        return EXIT_FAILURE;
    }

    /* После объявления своих интерфейсов сервер может начать ожидать входящие соединения. */
    Handle handle = INVALID_HANDLE;
    rc = ConnectWithClients(&handle);
    if (rc != rcOk)
    {
        fprintf(stderr, "[DynServer]: Client connection to Server failed, terminating.\n");
        return EXIT_FAILURE;
    }

    /* инициализация транспорта идентична echo_mod */
    NkKosTransport_Init(&transport, handle, NK_NULL, 0);

    /* Заготовим только константные части, на инициализации арены сэкономим */
    DynServer_entity_req req;
    DynServer_entity_res res;

    DynServer_entity entity;
    DynServer_entity_init(&entity, CreateIDynPingImpl());

    fprintf(stderr, "[DynServer] Cycle start\n");

    /* Начало рабочего цикла. */
    do
    {
        /* Очищаем поле запроса после предыдущего цикла */
        nk_req_reset(&req);
        /* Принимаем сообщение и диспетчируем по обработчикам */
        if (nk_transport_recv(&transport.base, &req.base_, NULL) == NK_EOK)
        {
            DynServer_entity_dispatch(&entity, &req.base_, NULL, &res.base_, NULL);
        }
        else
        {
            fprintf(stderr, "[DynServer]: nk_transport_recv error.\n");
        }
        /* Отправляем ответ клиенту */
        if (nk_transport_reply(&transport.base, &res.base_, NULL) != NK_EOK)
        {
            fprintf(stderr, "[DynServer]: nk_transport_reply error.\n");
        }
    }
    while (true);

    return EXIT_SUCCESS;
}
