#include <server_backend.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

DEBUG_STATIC int server_listen(uint32_t port, socklen_t * record_len);
void start_server(uint8_t thread_count, uint32_t port_num)
{

    int server_socket = server_listen(port_num, 0);
    if (-1 == server_socket)
    {
        return;
    }
    sleep(15);

    thpool_t * thpool = thpool_init(thread_count);
    if (NULL == thpool)
    {
        return;
    }

    close(server_socket);
    thpool_destroy(thpool);
}

DEBUG_STATIC int server_listen(uint32_t port, socklen_t * record_len)
{
    // Convert the port number into a string. The port number is already
    // verified, so we do not need to double-check it here
    char port_string[10];
    snprintf(port_string, 10, "%d", port);

    // Set up the hints structure to specify the parameters we want
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints = (struct addrinfo) {
        .ai_canonname = NULL,
        .ai_addr      = NULL,
        .ai_next      = NULL,
        .ai_socktype  = SOCK_STREAM,
        .ai_family    = AF_UNSPEC,        // Allows IPv4 or IPv6
        .ai_flags     = AI_PASSIVE,       // Use wildcard IP address (quad 0)
    };

    struct addrinfo * network_record_root;
    struct addrinfo * network_record;
    int sock_fd, enable_setsockopt;


    // Request socket structures that meet our criteria
    if (0 != getaddrinfo(NULL, port_string, &hints, &network_record_root))
    {

        debug_print_err("Unable to fetch socket structures: %s\n", strerror(errno));
        return -1;
    }

    /* Walk through returned list until we find an address structure
       that can be used to successfully create and bind a socket */

    enable_setsockopt = 1;
    for (network_record = network_record_root; network_record != NULL; network_record = network_record->ai_next) {

        // Using the current network record, attempt to create a socket fd
        // out of it. If it fails, grab the next one
        sock_fd = socket(network_record->ai_family,
                         network_record->ai_socktype,
                         network_record->ai_protocol);
        if (-1 == sock_fd)
        {
            continue;
        }

        // Attempt to modify the socket to be used for listening
        if (-1 == setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable_setsockopt, sizeof(enable_setsockopt)))
        {
            close(sock_fd);
            freeaddrinfo(network_record_root);
            return -1;
        }

        if (0 == bind(sock_fd, network_record->ai_addr, network_record->ai_addrlen))
        {
            break; // If successful bind, break we are done
        }

        // If we get here, then we failed to make the current network_record
        // work. Close the socker fd and fetch the next one
        close(sock_fd);
    }

    // If network_record is still NULL, then we failed to make all the
    // network records work
    if (NULL == network_record)
    {
        debug_print_err("%s\n", "Unable to bind to any socket");
        freeaddrinfo(network_record_root);
        return -1;
    }

    if (-1 == listen(sock_fd, BACK_LOG))
    {
        debug_print_err("LISTEN: %s\n", strerror(errno));
        freeaddrinfo(network_record_root);
        return -1;
    }

    char host[NI_MAXHOST], service[NI_MAXSERV];

    if (getnameinfo(network_record->ai_addr, network_record->ai_addrlen, host, NI_MAXHOST,
                    service, NI_MAXSERV, NI_NUMERICSERV) == 0)

    debug_print("[Server] Listening on %s:%s\n", host, service);

    // Set the size of the structure used. This could change based on the
    // ip version used
    if ((NULL != network_record) && (NULL != record_len))
    {
        *record_len = network_record->ai_addrlen;
    }
    freeaddrinfo(network_record_root);

    return (network_record == NULL) ? -1 : sock_fd;
}
