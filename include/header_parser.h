#ifndef JG_NETCALC_SRC_HEADER_PARSER_HEADER_PARSER_H_
#define JG_NETCALC_SRC_HEADER_PARSER_HEADER_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif //END __cplusplus
#include <stdint.h>
#include <calculation.h>

#define MAGIC_VALUE     0xDD77BB55
#define UNSOLVED_VAL    0x00
#define SOLVED_VAL      0x01
#define SIGNED_OUTPUT   0x01
#define UNSIGNED_OUTPUT 0x02

typedef enum {
    NET_HEADER_SIZE         = 4,
    NET_FILE_NAME_LEN       = 4,
    NET_TOTAL_PACKET_SIZE   = 8,
    NET_FILE_NAME           = 32,
    NET_MAX_HEADER_SIZE     = 48,
    NET_MAX_FILE_NAME       = 24
} NET_HEADER_BYTES;

typedef enum
{
    HEAD_MAGIC                  = 4,
    HEAD_FILEID                 = 8,
    HEAD_NUM_OF_EQU             = 8,
    HEAD_FLAGS                  = 1,
    HEAD_EQU_OFFSET             = 4,
    HEAD_NUM_OF_OPT_HEADERS     = 2

} EQU_FILE_HEADER_BYTES;

typedef enum
{
    UNSO_EQU_ID     = 4,
    UNSO_EQU        = 17,
    UNSO_FLAGS      = 1,
    UNSO_PADDING    = 10

} UNSOLVED_EQU_FORMAT_BYTES;

typedef enum
{
    SO_EQU_ID       = 4,
//    SO_FLAGS        = 1,
//    SO_TYPE         = 1,
//    SO_SOLUTION     = 8
} SOLVED_EQU_FORMAT_BYTES;


typedef enum
{
    L_OPERAND   = 8,
    OPERATOR    = 1,
    R_OPERAND   = 8
} SERIALIZED_EQU_FORMAT;


typedef struct unsolved_eq_t unsolved_eq_t;
typedef struct unsolved_eq_t
{
    uint32_t eq_id;
    uint8_t flags;
    uint64_t l_operand;
    uint8_t opt;
    uint64_t r_operand;
    unsolved_eq_t * next;
    solution_t * solution;
} unsolved_eq_t;

typedef struct equations_t
{
    uint32_t magic_id;
    uint64_t file_id;
    uint64_t number_of_eq;
    uint8_t flags;
    uint32_t offset;
    uint16_t num_of_opts;
    unsolved_eq_t * eqs;
    unsolved_eq_t * tail;
} equations_t;

typedef struct net_header_t
{
    uint32_t header_size;
    uint32_t name_len;
    uint64_t total_payload_size;
    uint8_t file_name[32];
} net_header_t;


equations_t * parse_stream(int fd);
net_header_t * read_header(int fd);
void free_equation(equations_t * eq);
void free_header(net_header_t * header);
uint64_t swap_byte_order(uint64_t val);

#ifdef __cplusplus
}
#endif //END __cplusplus
#endif //JG_NETCALC_SRC_HEADER_PARSER_HEADER_PARSER_H_
