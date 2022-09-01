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

class SignedCalcTestFixture : public ::testing::TestWithParam<std::tuple<uint64_t, uint8_t, uint64_t, int64_t, bool>>{};
class UnSignedCalcTestFixture : public ::testing::TestWithParam<std::tuple<uint64_t, uint8_t, uint64_t, uint64_t, bool>>{};

// Parameter test handles signed testing
TEST_P(SignedCalcTestFixture, TestingCalculations)
{
    auto [l_opr, opt, r_optr, expected_int, expected_succ] = GetParam();
    equation_t * eq = get_equation_struct(0, l_opr, opt, r_optr);

    if (1 == expected_succ)
    {
        EXPECT_EQ(eq->result, EQ_SOLVED) << l_opr << " (" << hexchar{opt} << ") " << r_optr << " = " << expected_int << "\n" << eq->error_msg;
        if (EQ_SOLVED == eq->result)
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

// Parameter test handles unsigned testing
TEST_P(UnSignedCalcTestFixture, TestingCalculations)
{
    auto [l_opr, opt, r_optr, expected_int, expected_succ] = GetParam();
    equation_t * eq = get_equation_struct(0, l_opr, opt, r_optr);

    if (1 == expected_succ)
    {
        EXPECT_EQ(eq->result, EQ_SOLVED) << l_opr << " (" << hexchar{opt} << ") " << r_optr << " = " << expected_int << "\n" << eq->error_msg;
        if (EQ_SOLVED == eq->result)
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
    SignedCalcTestFixture,
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
    SignedCalcTestFixture,
    ::testing::Values(
        std::make_tuple(10, 0x02, 10, 0, 1),
        std::make_tuple(INT64_MAX, 0x02, -10, 0, 0),
        std::make_tuple(INT64_MIN, 0x02, 10, 0, 0),
        std::make_tuple(1000, 0x02, 1000, 0, 1)
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
    MultiplicationTest,
    SignedCalcTestFixture,
    ::testing::Values(
        std::make_tuple(10, 0x03, 10, 100, 1),
        std::make_tuple(-1, 0x03, INT64_MIN, 0, 0),
        std::make_tuple(INT64_MIN, 0x03, -1, 0, 0),
        std::make_tuple(INT64_MAX, 0x03, 10, 0, 0),
        std::make_tuple(10, 0x03, INT64_MAX, 0, 0),
        std::make_tuple(1000, 0x03, 1000000000, 1000000000000, 1)
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
    DivisionTest,
    SignedCalcTestFixture,
    ::testing::Values(
        std::make_tuple(10, 0x04, 10, 1, 1),
        std::make_tuple(-1, 0x04, INT64_MIN, 0, 0),
        std::make_tuple(INT64_MIN, 0x04, -1, 0, 0),
        std::make_tuple(1, 0x04, 0, 0, 0),
        std::make_tuple(1000, 0x04, 100, 10, 1)
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
    ModulusTest,
    SignedCalcTestFixture,
    ::testing::Values(
        std::make_tuple(15, 0x05, 2, 1, 1),
        std::make_tuple(-1, 0x05, INT64_MIN, 0, 0),
        std::make_tuple(INT64_MIN, 0x05, -1, 0, 0),
        std::make_tuple(1, 0x05, 0, 0, 0),
        std::make_tuple(1000, 0x05, 100, 0, 1)
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
    LeftShiftTest,
    UnSignedCalcTestFixture,
    ::testing::Values(
        std::make_tuple(15, 0x06, 2, 60, 1),
        std::make_tuple(90000, 0x06, 50, 0x7E40000000000000, 1),
        std::make_tuple(0x7E40000000000000, 0x06, 1, 0xFC80000000000000, 1),
        std::make_tuple(0xFC80000000000000, 0x06, 1, 0xF900000000000000, 1),
        std::make_tuple(1, 0x06, 64, 1, 1)
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
    RightShiftTest,
    UnSignedCalcTestFixture,
    ::testing::Values(
        std::make_tuple(15, 0x07, 2, 3, 1),
        std::make_tuple(90000, 0x07, 50, 0, 1),
        std::make_tuple(0x518A1E7B7C14, 0x07, 1, 0x28C50F3DBE0A, 1),
        std::make_tuple(0x28C50F3DBE0A, 0x07, 1, 0x1462879EDF05, 1),
        std::make_tuple(1, 0x07, 64, 0, 1)
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
    AndTest,
    UnSignedCalcTestFixture,
    ::testing::Values(
        std::make_tuple(15, 0x08, 2, 2, 1),
        std::make_tuple(90000, 0x08, 50, 16, 1),
        std::make_tuple(0x8BA8568, 0x08, 0x8BA78C, 0x8A8508, 1),
        std::make_tuple(0, 0x08, 0, 0, 1)
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
    OrTest,
    UnSignedCalcTestFixture,
    ::testing::Values(
        std::make_tuple(15, 0x09, 2, 15, 1),
        std::make_tuple(90000, 0x09, 50, 90034, 1),
        std::make_tuple(0x9B5524, 0x09, 0x8B564, 0x9BF564, 1),
        std::make_tuple(0, 0x09, 0, 0, 1)
    ));
