#include <arg_parser.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <server.h>

DEBUG_STATIC void free_args(args_t * args);
DEBUG_STATIC uint32_t get_port(char * port);
DEBUG_STATIC uint8_t get_threads(char * thread);

static uint8_t str_to_long(char * str_num, long int * int_val);
/*!
 * @brief Free the arg_t object
 * @param args
 */
DEBUG_STATIC void free_args(args_t * args)
{
    free(args);
}

/*!
 * @brief Parse command line arguments to modify how th server is ran
 * @param argc Number of arguments  in the argv
 * @param argv Array of char arrays
 * @return Pointer to arg_t object if the args were valid otherwise NULL
 */
args_t * parse_args(int argc, char ** argv)
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
                if (0 == args->port)
                {
                    free_args(args);
                    return NULL;
                }
                break;
            case 'n':
                args->threads = get_threads(optarg);
                if (0 == args->threads)
                {
                    free_args(args);
                    return NULL;
                }
                break;
            case 'h':
                printf("Server listens on 0.0.0.0:31337 by default with "
                       "the option of modifying the port to listen on and the "
                       "number of threads to utilize.\n\n"
                       "-p  Port to listen to (default: 31337)\n"
                       "-n  Number of threads to use (default: 4)\n");
                free_args(args);
                return NULL;
            case '?':
                if ((optopt == 'p') || (optopt == 'n'))
                {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n",
                            optopt);
                }

                else if (isprint(optopt))
                {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                }
                else
                {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                }
                free_args(args);
                return NULL;
        }

    // If there is anything else to process, error out
    if (optind != argc)
    {
        free_args(args);
        return NULL;
    }
    return args;
}

/*!
 * @brief Conver the port string into a valid port int
 * @param port Pointer to the port string
 * @return uint32_t conversion of port; 0 if failure
 */
DEBUG_STATIC uint32_t get_port(char * port)
{
    long int converted_port = 0;
    int result = str_to_long(port, &converted_port);

    // If 0 is returned, return 0 indicating an error
    if (0 == result)
    {
        return 0;
    }

    // Return error if the value converted is larger than a port value
    if ((converted_port > MAX_PORT) || (converted_port < MIN_PORT))
    {
        return 0;
    }

    return (uint32_t)converted_port;
}

/*!
 * @brief Covert the string to an int
 * @param thread Pointer to the char to convert
 * @return uint32_t conversion of port; 0 if failure
 */
DEBUG_STATIC uint8_t get_threads(char * thread)
{
    long int converted_port = 0;
    int result = str_to_long(thread, &converted_port);

    // If 0 is returned, return 0 indicating an error
    if (0 == result)
    {
        return 0;
    }

    // Return error if the value converted is larger than a port value
    if ((converted_port > MAX_THREADS) || (converted_port < MIN_THREADS))
    {
        return 0;
    }

    return (uint8_t)converted_port;
}

/*!
 * @brief Function is mostly a replica of the strtol help menu to convert a
 * string into a long int
 *
 * @param str_num String to convert
 * @param int_val The result value
 * @return 0 if failure or 1 if successful
 */
static uint8_t str_to_long(char * str_num, long int * int_val)
{
    //To distinguish success/failure after call
    errno = 0;
    char * endptr = {0};
    *int_val = strtol(str_num, &endptr, 10);

    // Check for errors
    if (0 != errno)
    {
        return 0;
    }

    // No digits were found in the string to convert
    if (endptr == str_num)
    {
        return 0;
    }

    // If there are any extra characters, return 0
    if (* endptr != '\0')
    {
        return 0;
    }

    return 1;
}
