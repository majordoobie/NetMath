#include <gtest/gtest.h>
#include <calculation.h>

TEST(TestAllocs, TestAllocs)
{
    equation_t * eq = get_equation_struct(10, 0x01, 30);
    EXPECT_NE(eq, nullptr);
    free_equation_struct(eq);
}