#include <header_parser.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <utils.h>
#include <calculation.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

static int8_t read_stream(int fd, void * payload, void (free_func(void *)), void * buff, size_t size);

net_header_t * read_header(int fd)
{
    net_header_t * header = (net_header_t *)calloc(1, sizeof(net_header_t));
    if (UV_INVALID_ALLOC == verify_alloc((header)))
    {
        return NULL;
    }

    int res = read_stream(fd, header,(void (*)(void *))free_header, &header->header_size, NET_HEADER_SIZE);
    if (-1 == res)
    {
        return NULL;
    }
    header->header_size = ntohl(header->header_size);

    res = read_stream(fd, header,(void (*)(void *))free_header, &header->name_len, NET_FILE_NAME_LEN);
    if (-1 == res)
    {
        return NULL;
    }
    header->name_len = ntohl(header->name_len);

    res = read_stream(fd, header,(void (*)(void *))free_header, &header->total_payload_size, NET_TOTAL_PACKET_SIZE);
    if (-1 == res)
    {
        return NULL;
    }
    // Swap the byte order of the 64 bit value
    uint64_t val = header->total_payload_size;
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    val = (val << 32) | (val >> 32);
    header->total_payload_size = val;

    res = read_stream(fd, header,(void (*)(void *))free_header, &header->file_name, NET_FILE_NAME);
    if (-1 == res)
    {
        return NULL;
    }

    return header;
}

/*!
 * @brief Sequentially read the stream of bytes and fill the structs associated
 * with each section. If the read bytes does not match the amount required, free
 * the structure and return NULL;
 * @param fd File descriptor to read from
 * @return Pointer to the solution_t object
 */
equations_t * parse_stream(int fd)
{
    uint32_t magic_field = 0;
    read(fd, &magic_field, HEAD_MAGIC);
    if (MAGIC_VALUE != magic_field)
    {
        return NULL;
    }

    equations_t * eqs = (equations_t *)calloc(1, sizeof(equations_t));
    if (UV_INVALID_ALLOC == verify_alloc(eqs))
    {
        return NULL;
    }
    eqs->magic_id = magic_field;

    int res = read_stream(fd, eqs,(void (*)(void *))free_equation, &eqs->file_id, HEAD_FILEID);
    if (-1 == res)
    {
        return NULL;
    }

    res = read_stream(fd, eqs, (void (*)(void*))free_equation, &eqs->number_of_eq, HEAD_NUM_OF_EQU);
    if (-1 == res)
    {
        return NULL;
    }

    res = read_stream(fd, eqs, (void (*)(void*))free_equation, &eqs->flags, HEAD_FLAGS);
    if (-1 == res)
    {
        return NULL;
    }

    res = read_stream(fd, eqs, (void (*)(void*))free_equation, &eqs->offset, HEAD_EQU_OFFSET);
    if (-1 == res)
    {
        return NULL;
    }

    res = read_stream(fd, eqs, (void (*)(void*))free_equation, &eqs->num_of_opts, HEAD_NUM_OF_OPT_HEADERS);
    if (-1 == res)
    {
        return NULL;
    }

    for (int i = 0; i < eqs->number_of_eq; i++)
    {
        // Create the un_eq structure
        unsolved_eq_t * un_eq = (unsolved_eq_t *)calloc(1, sizeof(unsolved_eq_t));
        if (UV_INVALID_ALLOC == verify_alloc(un_eq))
        {
            free_equation(eqs);
            return NULL;
        }

        // Attach the un_eq structure to the solution_t structure at the tail
        if (NULL == eqs->tail)
        {
            eqs->eqs = un_eq;
            eqs->tail = un_eq;
        }
        else
        {
            eqs->tail->next = un_eq;
            eqs->tail = un_eq;
        }


        // Read from the stream all the sections for an unsolved equation
        res = read_stream(fd, eqs, (void (*)(void*))free_equation, &un_eq->eq_id, UNSO_EQU_ID);
        if (-1 == res)
        {
            return NULL;
        }

        res = read_stream(fd, eqs, (void (*)(void*))free_equation, &un_eq->flags, UNSO_FLAGS);
        if (-1 == res)
        {
            return NULL;
        }

        res = read_stream(fd, eqs, (void (*)(void*))free_equation, &un_eq->l_operand, L_OPERAND);
        if (-1 == res)
        {
            return NULL;
        }

        res = read_stream(fd, eqs, (void (*)(void*))free_equation, &un_eq->opt, OPERATOR);
        if (-1 == res)
        {
            return NULL;
        }

        res = read_stream(fd, eqs, (void (*)(void*))free_equation, &un_eq->r_operand, R_OPERAND);
        if (-1 == res)
        {
            return NULL;
        }

        // The last 10 bytes are just padding, use lseek to move the position
        lseek(fd, UNSO_PADDING, SEEK_CUR);
    }

    return eqs;
}

void free_equation(equations_t * eq)
{
    if (NULL != eq->eqs)
    {
        unsolved_eq_t * eqs = eq->eqs;
        void * free_tmp;
        while (NULL != eqs)
        {
            // Free the result if there is one defined
            if (NULL != eqs->solution)
            {
                free_equation_struct(eqs->solution);
            }
            free_tmp = eqs;
            eqs = eqs->next;
            free(free_tmp);
        }
    }
    free(eq);
}

void free_header(net_header_t * header)
{
    free(header);
}

/*!
 * @brief Function repeatedly reads from the file descriptor passed in and
 * writes the data read into the buffer provided
 * @param fd File descriptor read
 * @param eq Equation structure object
 * @param buff Buffer to write the read data to
 * @param size Number of bytes to read from the file descriptor
 * @return 0 if read was successful, -1 if invalid. Free eq if invalid before
 * returning
 */
static int8_t read_stream(int fd, void * payload, void (free_func(void *)), void * buff, size_t size)
{
    ssize_t read_bytes;
    read_bytes = read(fd, buff, size);
    if (-1 == read_bytes)
    {
        debug_print_err("Unable to read from fd: %s\n", strerror(errno));
        free_func(payload);
        return -1;
    }
    if (size != read_bytes)
    {
        debug_print_err("[HEADER READ] Read bytes did not match expected "
                        "bytes. Read %zd our of %zu\n", read_bytes, size);
        free_func(payload);
        return -1;
    }
    return 0;
}
