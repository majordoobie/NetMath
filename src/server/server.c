#include <server.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

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
DEBUG_STATIC uint32_t get_port(char * port);

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


    opterr = 0;
    int c = 0;

    while ((c = getopt(argc, argv, "p:n:h")) != -1)
        switch (c)
        {
            case 'p':
                args->port = get_port(optarg);
                break;
            case 'n':
//                args->threads = get_port(optarg);

                break;
            case '?':
                if (optopt == 'c')
                    fprintf(stderr,
                            "Option -%c requires an argument.\n",
                            optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
            default:
                abort();
        }



    return NULL;
}

DEBUG_STATIC uint32_t get_port(char * port)
{
    return 0;
}
