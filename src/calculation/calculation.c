#include <calculation.h>
#include <stdlib.h>

// This enum is used for the eval union return value
typedef enum eq_eval_type_t
{
    EQ_VAL_SIGNED,      // Eval result is of type signed
    EQ_VAL_UNSIGNED     // Eval result is of type unsigned
} eq_eval_type_t;

// Structure contains the data needed to resolve the equation
typedef struct equation_t
{
    char * error_msg;       // Message indicating the error
    uint64_t l_operand;     // Left operand
    uint64_t r_operand;     // Right operand
    uint64_t evaluation;    // Eval result
    eq_eval_type_t sign;    // Whether the result is singed or unsigned
    uint8_t opt;            // Operator byte code
} equation_t;

/*!
 * @brief Allocate memory for an equation object and return it containing the
 * operator and operands passed in.
 * @param l_operand Left operand
 * @param opt Operator byte code
 * @param r_operand Right Operand
 * @return Pointer to an allocated equation object or NULL if unable to allocate
 * memory
 */
equation_t * get_equation_struct(uint64_t l_operand, uint8_t opt, uint64_t r_operand)
{
    equation_t * equation = (equation_t *)malloc(sizeof(equation_t));
    if (UV_INVALID_ALLOC == verify_alloc(equation))
    {
        return NULL;
    }

    *equation = (equation_t) {
        .error_msg  = NULL,
        .l_operand  = l_operand,
        .r_operand  = r_operand,
        .evaluation = 0,
        .opt        = opt,
        .sign       = EQ_VAL_SIGNED,
    };

    return equation;
}

/*!
 * @brief Free the equation structure
 * @param equation Pointer to the equation object
 */
void free_equation_struct(equation_t * equation)
{
    if (NULL != equation->error_msg)
    {
        free(equation->error_msg);
    }
    free(equation);
}
