#include <gtest/gtest.h>
#include <calculation.h>

TEST(Test1, Test1)
{
    print_it();
    char * h = (char *)malloc(sizeof(char) * 2);
    h[0] = 'a';
    h[1] = 0;
    free(h);

}