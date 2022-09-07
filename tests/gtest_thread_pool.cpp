#include <gtest/gtest.h>
#include <thread_pool.h>

void stack(void)
{
    printf("HI!\n");
}

TEST(TestAllocDestroy, TestAllocDestroy)
{
    thpool_t * thpool = thpool_init(4);
    sleep(5);
    thpool_enqueue_job(thpool, reinterpret_cast<void (*)(void *)>(stack), NULL);
    sleep(10);
    thpool_destroy(thpool);
}