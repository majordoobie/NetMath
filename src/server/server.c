#include <server.h>
#include <stdint.h>
#include <stdlib.h>

// Enable private function to be used publicly when in debug release mode
#ifdef NDEBUG
#define DEBUG_STATIC static
#else
#define DEBUG_STATIC
#endif

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

DEBUG_STATIC args_t * parse_args(int argc, char ** argv);
DEBUG_STATIC void free_args(args_t * args);

DEBUG_STATIC void free_args(args_t * args)
{
    free(args);
}

DEBUG_STATIC args_t * parse_args(int argc, char ** argv)
{
    args_t * args = (args_t *)malloc(sizeof(args_t));
    if (UV_INVALID_ALLOC == verify_alloc(args))
    {
        return NULL;
    }
    *args = (args_t){
        .port       = DEFAULT_PORT,
        .threads    = DEFAULT_THREADS
    };

    // If not additional arguments have been specified, return the default;
    if (1 == argc)
    {
        return args;
    }

    return NULL;
}
