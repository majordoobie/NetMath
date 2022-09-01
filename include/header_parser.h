#ifndef JG_NETCALC_SRC_HEADER_PARSER_HEADER_PARSER_H_
#define JG_NETCALC_SRC_HEADER_PARSER_HEADER_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif //END __cplusplus

#define MAGIC_VALUE     0xDD77BB55
#define UNSOLVED_VAL    0x00
#define SOLVED_VAL      0x01
#define SIGNED_OUTPUT   0x01
#define UNSIGNED_OUTPUT 0x02

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
    SO_FLAGS        = 1,
    SO_TYPE         = 1,
    SO_SOLUTION     = 8
} SOLVED_EQU_FORMAT_BYTES;


typedef enum
{
    L_OPERAND   = 8,
    OPERATOR    = 1,
    R_OPERAND   = 8
} SERIALIZED_EQU_FORMAT;

void print(void);

#ifdef __cplusplus
}
#endif //END __cplusplus
#endif //JG_NETCALC_SRC_HEADER_PARSER_HEADER_PARSER_H_
