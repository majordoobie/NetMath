#include <calculation.h>
#include <stdint.h>
#include <stdlib.h>



typedef enum eq_result_t
{
    EQ_SOLVED,     // Equation was successfully solved
    EQ_FAILURE,    // Equation resulted in a failure
    EQ_UNSOLVED    // Equation has not been attempted yet

} eq_result_t;

typedef struct equation_t
{
    uint64_t l_operand;
    uint64_t r_operand;
    union
    {
        uint64_t u_evaluation;
        int64_t s_evaluation;
    } evaluation;
    uint8_t operator;
    char * error_msg;
} equation_t;

equation_t * get_equation_struct(uint64_t l_operand, uint8_t operator, uint64_t r_operand)
{
    equation_t * equation = (equation_t *)malloc(sizeof(equation_t));

}
