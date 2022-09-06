#include <gtest/gtest.h>
#include <thread_pool.h>

TEST(TestAllocDestroy, TestAllocDestroy)
{
    thpool_t * thpool = thpool_init(4);
    thpool_destroy(thpool);
}