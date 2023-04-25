#ifndef ECHO_MOD_DYN_COMMON_H
#define ECHO_MOD_DYN_COMMON_H

#include <stdio.h>
#include <stdlib.h>

/* Библиотеки для работы с транспортом. */
#include <coresrv/nk/transport-kos.h>
#include <coresrv/handle/handle_api.h>
#include <coresrv/cm/cm_api.h>
#include <coresrv/ns/ns_api.h>
#include <coresrv/thread/thread_api.h>

#include <assert.h>
#include <nk/types.h>
#include <rtl/minmax.h>

#define NK_USE_UNQUALIFIED_NAMES    // позволяет выписывать неполные имена компонентов. См h-файлы в папке build

#define UCORE_STRING_SIZE 1024      // константы для сервисов Ns* и Kn*
#define NS_WAIT_TIME_MS 10000
#define RISEUP_TIMEOUT_MSEC 1000U
#define TIMEOUT_MSEC 5000U

#define EXAMPLE_VALUE_TO_SEND 777;

static const char InterfaceType[] = "echo_mod_dyn.IDynPing";

#endif