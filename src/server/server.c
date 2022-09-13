#include <server.h>
#include <thread_pool.h>


DEBUG_STATIC void free_args(args_t * args);


int main(int argc, char ** argv)
{
    args_t * args = parse_args(argc, argv);
    if (NULL == args)
    {
        exit(-1);
    }

    free_args(args);
}


