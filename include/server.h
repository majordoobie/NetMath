#ifndef JG_NETCALC_SRC_SERVER_SERVER_H_
#define JG_NETCALC_SRC_SERVER_SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif //END __cplusplus

#include <utils.h>
#include <stdint.h>

typedef enum
{
    MAX_THREADS = UINT8_MAX,
    MIN_PORT    = 1,
    MAX_PORT    = 0xFFFF
} server_defaults_t;

#ifdef __cplusplus
}
#endif // END __cplusplus

#endif //JG_NETCALC_SRC_SERVER_SERVER_H_
