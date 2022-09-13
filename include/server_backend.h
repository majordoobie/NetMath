#ifndef JG_NETCALC_SRC_SERVER_SERVER_BACKEND_H_
#define JG_NETCALC_SRC_SERVER_SERVER_BACKEND_H_
#ifdef __cplusplus
extern "C" {
#endif //END __cplusplus
#include <utils.h>
#include <stdint.h>
#include <thread_pool.h>
typedef enum
{
    MAX_THREADS = UINT8_MAX,
    MIN_THREADS = 1,
    MIN_PORT    = 1,
    MAX_PORT    = 0xFFFF,
    BACK_LOG    = 1024
} server_defaults_t;

void start_server(uint8_t thread_count, uint32_t port_num);

#ifdef __cplusplus
}
#endif // END __cplusplus
#endif //JG_NETCALC_SRC_SERVER_SERVER_BACKEND_H_
