#include <server_backend.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <header_parser.h>
#include <stdlib.h>
#include <signal.h>
#include <stdatomic.h>

DEBUG_STATIC int server_listen(uint32_t port, socklen_t * record_len);
DEBUG_STATIC void serve_client(void * sock);
static int get_ip_port(struct sockaddr * addr, socklen_t addr_size, char * host, char * port);
static void signal_handler(int signal);
static void error_reply(void * sock_oid, net_header_t * header);
static void serialize_header(net_header_t * header,
                             uint8_t * buffer,
                             size_t buffer_size);

void error_reply(void * sock_oid, net_header_t * header);
// Atomic flag is used to control the server running
static atomic_flag server_run;

/*!
 * @brief Start the EQU server on the specified ports. The function will
 * also spawn a thread pool object with the thread_count of the value
 * provided
 *
 * @param thread_count Number of threads to spawn in the thread pool
 * @param port_num Port number to listen on
 */
void start_server(uint8_t thread_count, uint32_t port_num)
{

    // Make the server start listening
    int server_socket = server_listen(port_num, 0);
    if (-1 == server_socket)
    {
        return;
    }

    // Initialize the thread pool for the connections of clients
    thpool_t * thpool = thpool_init(thread_count);
    if (NULL == thpool)
    {
        close(server_socket);
        return;
    }


    // Set up SIGINT signal handling
	struct sigaction signal_action;
	memset(&signal_action, 0, sizeof(signal_action));
	signal_action.sa_handler = signal_handler;

	// Make the system call to change the action taken by the process on receipt
	// of the signal
	if (-1 == (sigaction(SIGINT, &signal_action, NULL)))
	{
        debug_print_err("%s\n", "Unable to set up signal handler");
        close(server_socket);
        thpool_destroy(thpool);
        return;
	}

    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_storage);

    atomic_flag_test_and_set(&server_run);
    while (atomic_flag_test_and_set(&server_run))
    {
        // Clear the client_addr before the next connection
        memset(&client_addr, 0, addr_size);
        int client_fd = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (-1 == client_fd)
        {
            debug_print_err("Failed to accept: %s\n", strerror(errno));
        }
        else
        {
            char host[NI_MAXHOST];
            char service[NI_MAXSERV];
            if (0 == get_ip_port((struct sockaddr *)&client_addr, addr_size, host, service))
            {
                debug_print("[SERVER] Received connection from %s:%s\n", host, service);
            }
            else
            {
                printf("[SERVER] Received connection from unknown peer\n");
            }
            int * fd = (int *)malloc(sizeof(int));
            if (UV_INVALID_ALLOC == verify_alloc((fd)))
            {
                debug_print_err("[SERVER] Unable to allocate memory for fd "
                                "for connection %s:%s\n", host, service);
                close(client_fd);
            }
            else
            {
                *fd = client_fd;
                thpool_enqueue_job(thpool, serve_client, fd);
            }
        }
    }

    // Wait for all the jobs to finish
    thpool_wait(thpool);

    // Close the server
    close(server_socket);
    thpool_destroy(thpool);
}

/*!
 * @brief This function is a thread callback. As soon as the server
 * receives a connection, the server will queue the connection into the
 * threadpool job queue. Once the job is dequeued, the thread will execute
 * this callback function. This is where the individual files are parsed
 * and returned to the client.
 *
 * @param sock_void Void pointer containing the connection file descriptor
 */
DEBUG_STATIC void serve_client(void * sock_void)
{
    int client_sock = *(int *)sock_void;

    net_header_t * header = read_header(client_sock);
    if (NULL == header)
    {
        close(client_sock);
        free(sock_void);
        return;
    }

    printf("[SERVER THREAD]\n\t\tHeader size: %d\n\t\tName Len: %d\n\t\tTotal Packets: %lu\n\t\tFile Name: "
           "%s\n", header->header_size, header->name_len, header->total_payload_size,
           header->file_name);

    if (NET_MAX_HEADER_SIZE != header->header_size)
    {
        debug_print("[SERVER THREAD] Header size does not match the expected "
                    "value of %d. Read %u instead\n", NET_MAX_HEADER_SIZE,
                    header->header_size);

        error_reply(sock_void, header);
        return;

    }

    if (header->name_len > NET_MAX_FILE_NAME)
    {
        debug_print("[SERVER THREAD] File name length exceeds the size limit "
                    "of %d. The length is set to %d\n", NET_MAX_FILE_NAME,
                    header->name_len);
        error_reply(sock_void, header);
        return;
    }

    close(client_sock);
    free(sock_void);
    free_header(header);
}

static void error_reply(void * sock_void, net_header_t * header)
{
    int client_sock = *(int *)sock_void;
    header->name_len = 0;
    memset(header->file_name, 0, NET_MAX_FILE_NAME);

    uint8_t * buffer = (uint8_t *)calloc(NET_MAX_HEADER_SIZE, sizeof(uint8_t));
    if (UV_INVALID_ALLOC == verify_alloc(buffer))
    {
        return;
    }

    serialize_header(header, buffer, NET_MAX_HEADER_SIZE);
    ssize_t res = write(client_sock, buffer, NET_MAX_HEADER_SIZE);
    if (-1 == res)
    {
        debug_print_err("[SERVER THREAD] Error writting %s\n", strerror(errno));
    }

    free(buffer);
    free_header(header);
    close(client_sock);
    free(sock_void);
}

static void serialize_header(net_header_t * header, uint8_t * buffer, size_t buffer_size)
{
    size_t offset = 0;
    uint32_t buff = 0;

    buff = htonl(header->header_size);
    memcpy(buffer, &buff, NET_HEADER_SIZE);
    offset += NET_HEADER_SIZE;

    buff = htonl(header->name_len);
    memcpy(buffer + offset, &buff, NET_FILE_NAME_LEN);
    offset += NET_FILE_NAME_LEN;

    uint64_t buff64 = swap_byte_order(header->total_payload_size);
    memcpy(buffer + offset, &buff64, NET_TOTAL_PACKET_SIZE);
    offset += NET_TOTAL_PACKET_SIZE;

    memcpy(buffer + offset, &header->file_name, NET_FILE_NAME);
}

/*!
 * @brief Start listening on the port provided on quad 0s. The file descriptor
 * for the socket is returned
 * @param port Port number to listen on
 * @param record_len Populated with the size of the sockaddr. This value is
 * dependant on the structure used. IPv6 structures are larger.
 * @return Either -1 for failure or 0 for success
 */
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

        debug_print_err("[SERVER] Unable to fetch socket structures: %s\n", strerror(errno));
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

        debug_print("[SERVER] bind error: %s. Trying next record\n", strerror(errno));

        // If we get here, then we failed to make the current network_record
        // work. Close the socker fd and fetch the next one
        close(sock_fd);
    }

    // If network_record is still NULL, then we failed to make all the
    // network records work
    if (NULL == network_record)
    {
        debug_print_err("%s\n", "[SERVER] Unable to bind to any socket");
        freeaddrinfo(network_record_root);
        return -1;
    }

    if (-1 == listen(sock_fd, BACK_LOG))
    {
        debug_print_err("[SERVER] LISTEN: %s\n", strerror(errno));
        freeaddrinfo(network_record_root);
        return -1;
    }

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    if (0 == get_ip_port(network_record->ai_addr, network_record->ai_addrlen, host, service))
    {
        debug_print("[SERVER] Listening on %s:%s\n", host, service);
    }
    else
    {
        debug_print("%s\n", "[SERVER] Unknown ip and port listening on");
    }


    // Set the size of the structure used. This could change based on the
    // ip version used
    if ((NULL != network_record) && (NULL != record_len))
    {
        *record_len = network_record->ai_addrlen;
    }
    freeaddrinfo(network_record_root);

    return (network_record == NULL) ? -1 : sock_fd;
}

static int get_ip_port(struct sockaddr * addr, socklen_t addr_size, char * host, char * port)
{
    return getnameinfo(addr, addr_size, host, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICSERV);

}

static void signal_handler(int signal)
{
    debug_print("%s\n", "[SERVER] Gracefully shutting down...");
    atomic_flag_clear(&server_run);
}
