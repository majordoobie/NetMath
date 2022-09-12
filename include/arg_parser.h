#ifndef JG_NETCALC_INCLUDE_ARG_PARSER_H_
#define JG_NETCALC_INCLUDE_ARG_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
#include <stdlib.h>
#include <utils.h>

typedef enum
{
    MAX_THREADS = UINT8_MAX,
    MIN_THREADS = 1,
    MIN_PORT    = 1,
    MAX_PORT    = 0xFFFF
} server_defaults_t;

typedef enum
{
    DEFAULT_PORT    = 31337,
    DEFAULT_THREADS = 4
} args_default_t;

typedef struct args_t
{
    uint32_t port;
    uint8_t threads;
} args_t;

args_t * parse_args(int argc, char ** argv);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //JG_NETCALC_INCLUDE_ARG_PARSER_H_
