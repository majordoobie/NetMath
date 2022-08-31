#ifndef JG_NETCALC_SRC_CALCULATION_CALCULATION_H_
#define JG_NETCALC_SRC_CALCULATION_CALCULATION_H_

#ifdef __cplusplus
extern "C" {
#endif //END __cplusplus

#include <stdint.h>
#include <utils.h>
// This enum is used for the eval union return value
typedef enum eq_eval_type_t
{
    EQ_VAL_SIGNED,      // Eval result is of type signed
    EQ_VAL_UNSIGNED     // Eval result is of type unsigned
} eq_eval_type_t;

typedef enum eq_result_t
{
    EQ_SOLVED,     // Equation was successfully solved
    EQ_FAILURE,    // Equation resulted in a failure
    EQ_UNSOLVED    // Equation has not been attempted yet
} eq_result_t;

// Structure contains the data needed to resolve the equation
typedef struct equation_t
{
    uint32_t eq_id;         // Equation ID provided by the spec
    char * error_msg;       // Message indicating the error
    uint64_t l_operand;     // Left operand
    uint64_t r_operand;     // Right operand
    uint64_t evaluation;    // Eval result
    eq_eval_type_t sign;    // Whether the result is singed or unsigned
    eq_result_t result;     // Conclusion of the operation
    uint8_t opt;            // Operator byte code
} equation_t;

equation_t * get_equation_struct(uint32_t equation_id,
                                 uint64_t l_operand,
                                 uint8_t opt,
                                 uint64_t r_operand);
void free_equation_struct(equation_t * equation);

#ifdef __cplusplus
}
#endif //END __cplusplus
#endif //JG_NETCALC_SRC_CALCULATION_CALCULATION_H_
