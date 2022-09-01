#include <calculation.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



static void resolve_equation(equation_t * eq);
static void add_callback(equation_t * eq);
static void sub_callback(equation_t * eq);
static void mul_callback(equation_t * eq);
static void div_callback(equation_t * eq);
static void mod_callback(equation_t * eq);
static void s_left_callback(equation_t * eq);
static void s_right_callback(equation_t * eq);
static void and_callback(equation_t * eq);
static void or_callback(equation_t * eq);
static void xor_callback(equation_t * eq);
static void r_left_callback(equation_t * eq);
static void r_right_callback(equation_t * eq);

static void unknown_callback(equation_t * eq);

#define MAX_BITS 64

/*!
 * @brief Allocate memory for an equation object and return it containing the
 * operator and operands passed in.
 * @param l_operand Left operand
 * @param opt Operator byte code
 * @param r_operand Right Operand
 * @return Pointer to an allocated equation object or NULL if unable to allocate
 * memory
 */
equation_t * get_equation_struct(uint32_t equation_id,
                                 uint64_t l_operand,
                                 uint8_t opt,
                                 uint64_t r_operand)
{
    equation_t * equation = (equation_t *)malloc(sizeof(equation_t));
    if (UV_INVALID_ALLOC == verify_alloc(equation))
    {
        return NULL;
    }

    *equation = (equation_t) {
        .eq_id      = equation_id,
        .error_msg  = NULL,
        .l_operand  = l_operand,
        .r_operand  = r_operand,
        .evaluation = 0,
        .opt        = opt,
        .sign       = EQ_VAL_UNSIGNED,
        .result     = EQ_UNSOLVED,
    };

    resolve_equation(equation);
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

static void resolve_equation(equation_t * eq)
{
    switch(eq->opt)
    {
        case 0x01:
            add_callback(eq);
            break;
        case 0x02:
            sub_callback(eq);
            break;
        case 0x03:
            mul_callback(eq);
            break;
        case 0x04:
            div_callback(eq);
            break;
        case 0x05:
            mod_callback(eq);
            break;
        case 0x06:
            s_left_callback(eq);
            break;
        case 0x07:
            s_right_callback(eq);
            break;
        case 0x08:
            and_callback(eq);
            break;
        case 0x09:
            or_callback(eq);
            break;
        case 0x0a:
            xor_callback(eq);
            break;
        case 0x0b:
            r_left_callback(eq);
            break;
        case 0x0c:
            r_right_callback(eq);
            break;
        default:
            unknown_callback(eq);
            break;
    }
}

static void add_callback(equation_t * eq)
{
    int64_t l_operand = (int64_t)eq->l_operand;
    int64_t r_operand = (int64_t)eq->r_operand;

    if ((r_operand > 0) && (l_operand > (INT64_MAX - r_operand)))
    {
        eq->error_msg = strdup("Overflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }
    if ((r_operand < 0) && (l_operand < (INT64_MIN - r_operand)))
    {
        eq->error_msg = strdup("Underflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }

    eq->evaluation = (uint64_t)(l_operand + r_operand);
    eq->sign = EQ_VAL_SIGNED;
    eq->result = EQ_SOLVED;
    return;
}

static void sub_callback(equation_t * eq)
{
    int64_t l_operand = (int64_t)eq->l_operand;
    int64_t r_operand = (int64_t)eq->r_operand;


    if ((r_operand < 0) && (l_operand > (INT64_MAX + r_operand)))
    {
        eq->error_msg = strdup("Overflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }
    if ((r_operand > 0) && (l_operand < (INT64_MIN + r_operand)))
    {
        eq->error_msg = strdup("Underflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }

    eq->evaluation = (uint64_t)(l_operand - r_operand);
    eq->sign = EQ_VAL_SIGNED;
    eq->result = EQ_SOLVED;
    return;
}

static void mul_callback(equation_t * eq)
{
    int64_t l_operand = (int64_t)eq->l_operand;
    int64_t r_operand = (int64_t)eq->r_operand;

    if ((-1 == l_operand) && (INT64_MIN == r_operand))
    {
        eq->error_msg = strdup("Overflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }
    if ((-1 == r_operand) && (INT64_MIN == l_operand))
    {
        eq->error_msg = strdup("Overflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }
    if ((0 != r_operand) && (l_operand > (INT64_MAX / r_operand)))
    {
        eq->error_msg = strdup("Overflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }
    if ((0 != r_operand) && (l_operand < (INT64_MIN / r_operand)))
    {
        eq->error_msg = strdup("Underflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }

    eq->evaluation = (uint64_t)(l_operand * r_operand);
    eq->sign = EQ_VAL_SIGNED;
    eq->result = EQ_SOLVED;
    return;
}

static void div_callback(equation_t * eq)
{
    int64_t l_operand = (int64_t)eq->l_operand;
    int64_t r_operand = (int64_t)eq->r_operand;

    if (0 == r_operand)
    {
        eq->error_msg = strdup("Division by zero error\n");
        eq->result = EQ_FAILURE;
        return;
    }

    if ((INT64_MIN == l_operand) && (-1 == r_operand))
    {
        eq->error_msg = strdup("Overflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }

    if ((INT64_MIN == r_operand) && (-1 == l_operand))
    {
        eq->error_msg = strdup("Overflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }

    eq->evaluation = (uint64_t)(l_operand / r_operand);
    eq->sign = EQ_VAL_SIGNED;
    eq->result = EQ_SOLVED;
    return;
}

static void mod_callback(equation_t * eq)
{
    int64_t l_operand = (int64_t)eq->l_operand;
    int64_t r_operand = (int64_t)eq->r_operand;

    if (0 == r_operand)
    {
        eq->error_msg = strdup("Division by zero error\n");
        eq->result = EQ_FAILURE;
        return;
    }

    if ((INT64_MIN == l_operand) && (-1 == r_operand))
    {
        eq->error_msg = strdup("Overflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }

    if ((INT64_MIN == r_operand) && (-1 == l_operand))
    {
        eq->error_msg = strdup("Overflow detected for operation\n");
        eq->result = EQ_FAILURE;
        return;
    }

    eq->evaluation = (uint64_t)(l_operand % r_operand);
    eq->sign = EQ_VAL_SIGNED;
    eq->result = EQ_SOLVED;
    return;
}

static void s_left_callback(equation_t * eq)
{
    uint64_t l_operand = eq->l_operand;
    uint64_t r_operand = eq->r_operand;

    // If r_operand is higher than max bits, then molus it to "rotate" around
    if (r_operand > (MAX_BITS - 1))
    {
        r_operand = r_operand % MAX_BITS;
    }
    eq->evaluation = l_operand << r_operand;
    eq->result = EQ_SOLVED;
    return;
}

static void s_right_callback(equation_t * eq)
{
    uint64_t l_operand = eq->l_operand;
    uint64_t r_operand = eq->r_operand;

    // If r_operand is higher than max_bits, then set it to max bits which
    // will wipe the value to 0
    if (r_operand > (MAX_BITS - 1))
    {
        r_operand = MAX_BITS - 1;
    }
    eq->evaluation = l_operand >> r_operand;
    eq->result = EQ_SOLVED;
    return;
}

static void and_callback(equation_t * eq)
{
    eq->evaluation = eq->l_operand & eq->r_operand;
    eq->result = EQ_SOLVED;
    return;
}

static void or_callback(equation_t * eq)
{
    eq->evaluation = eq->l_operand | eq->r_operand;
    eq->result = EQ_SOLVED;
    return;
}

static void xor_callback(equation_t * eq)
{
    eq->evaluation = eq->l_operand ^ eq->r_operand;
    eq->result = EQ_SOLVED;
    return;
}

static void r_left_callback(equation_t * eq)
{
    uint64_t l_operand = eq->l_operand;
    uint64_t r_operand = eq->r_operand;

    if (r_operand > (MAX_BITS - 1))
    {
        r_operand = r_operand % MAX_BITS;
    }

    if (0 != r_operand)
    {
        eq->evaluation = ((l_operand << r_operand) | (l_operand >> (MAX_BITS - r_operand)));
    }
    else
    {
        eq->evaluation = l_operand;
    }

    eq->result = EQ_SOLVED;
    return;
}

static void r_right_callback(equation_t * eq)
{
    uint64_t l_operand = eq->l_operand;
    uint64_t r_operand = eq->r_operand;

    if (r_operand > (MAX_BITS - 1))
    {
        r_operand = r_operand % MAX_BITS;
    }

    if (0 != r_operand)
    {
        eq->evaluation = ((l_operand >> r_operand) | (l_operand << (MAX_BITS - r_operand)));
    }
    else
    {
        eq->evaluation = l_operand;
    }
    eq->result = EQ_SOLVED;
    return;
}


static void unknown_callback(equation_t * eq)
{
    char error[30];
    snprintf(error, 30, "Unknown operand %#02x\n", eq->opt);
    eq->error_msg = strdup(error);
    eq->result = EQ_FAILURE;
    return;
}
