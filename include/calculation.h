#ifndef JG_NETCALC_SRC_CALCULATION_CALCULATION_H_
#define JG_NETCALC_SRC_CALCULATION_CALCULATION_H_

#ifdef __cplusplus
extern "C" {
#endif //END __cplusplus

#include <stdint.h>
#include <utils.h>
typedef enum eq_result_t
{
    EQ_SOLVED,     // Equation was successfully solved
    EQ_FAILURE,    // Equation resulted in a failure
    EQ_UNSOLVED    // Equation has not been attempted yet
} eq_result_t;


typedef struct equation_t equation_t;
equation_t * get_equation_struct(uint64_t l_operand, uint8_t opt, uint64_t r_operand);
void free_equation_struct(equation_t * equation);

#ifdef __cplusplus
}
#endif //END __cplusplus
#endif //JG_NETCALC_SRC_CALCULATION_CALCULATION_H_
