#include <gtest/gtest.h>
#include <calculation.h>

struct hexchar
{
    char c;
};

std::ostream& operator<<(std::ostream& os, hexchar h)
{
    auto f = os.flags();
    os << "0x";
    os << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
    os << (unsigned int)(unsigned char)h.c;
    os.flags(f);
    return os;
}

TEST(TestAllocs, TestAllocs)
{
    equation_t * eq = get_equation_struct(0, 10, 0x01, 30);

    EXPECT_NE(eq, nullptr);
    free_equation_struct(eq);
}

class CalculationFixture :public ::testing::TestWithParam<std::tuple<uint64_t, uint8_t, uint64_t, int, bool>>{};

TEST_P(CalculationFixture, TestingCalculations)
{
    auto [l_opr, opt, r_optr, expected_int, expected_succ] = GetParam();
    equation_t * eq = get_equation_struct(0, l_opr, opt, r_optr);

    if (1 == expected_succ)
    {
        EXPECT_EQ(eq->result, EQ_SOLVED) << l_opr << " (" << hexchar{opt} << ") " << r_optr << " = " << expected_int << "\n" << eq->error_msg;
        if (1 == eq->result)
        {
            EXPECT_EQ(eq->evaluation, expected_int) << l_opr << " (" << hexchar{opt} << ") " << r_optr << " = " << expected_int;
        }
    }
    else
    {
        EXPECT_EQ(eq->result, EQ_FAILURE);
    }
    free_equation_struct(eq);
}

/*
 * Param Order
 * l_operand
 * opt
 * r_operand
 * expected_result
 * expected_err
 */
INSTANTIATE_TEST_SUITE_P(
        AdditionTests,
        CalculationFixture,
        ::testing::Values(
            std::make_tuple(10, 0x01, 10, 20, 1),
            std::make_tuple(INT64_MAX, 0x01, 10, 0, 0),
            std::make_tuple(INT64_MIN, 0x01, -10, 0, 0),
            std::make_tuple(1000, 0x01, 1000, 2000, 1)
        ));

/*
 * Param Order
 * l_operand
 * opt
 * r_operand
 * expected_result
 * expected_err
 */
INSTANTIATE_TEST_SUITE_P(
    SubtractionTest,
    CalculationFixture,
    ::testing::Values(
        std::make_tuple(10, 0x02, 10, 0, 1),
        std::make_tuple(INT64_MAX, 0x02, -10, 0, 0),
        std::make_tuple(INT64_MIN, 0x02, 10, 0, 0),
        std::make_tuple(1000, 0x02, 1000, 0, 1)
    ));
