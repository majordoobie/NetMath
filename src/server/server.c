#include <server.h>
#include <thread_pool.h>

// Enable private function to be used publicly when in debug release mode
#ifdef NDEBUG
#define DEBUG_STATIC static
#else
#define DEBUG_STATIC
#endif

DEBUG_STATIC void free_args(args_t * args);
DEBUG_STATIC void start_server(uint8_t thread_count, uint32_t port_num);


int main(int argc, char ** argv)
{
    args_t * args = parse_args(argc, argv);
    if (NULL == args)
    {
        exit(-1);
    }

    free_args(args);
}

DEBUG_STATIC void start_server(uint8_t thread_count, uint32_t port_num)
{
    thpool_t * thpool = thpool_init(thread_count);
    if (NULL == thpool)
    {
        return;
    }



    thpool_destroy(thpool);
}

