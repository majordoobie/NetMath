#include <gtest/gtest.h>
#include <thread_pool.h>

void stack(void)
{
    printf("HI!\n");
}

TEST(TestAllocDestroy, TestAllocDestroy)
{
    thpool_t * thpool = thpool_init(4);

    thpool_enqueue_job(thpool, (void (*)(void *))stack, NULL);
    thpool_enqueue_job(thpool, (void (*)(void *))stack, NULL);
    thpool_enqueue_job(thpool, (void (*)(void *))stack, NULL);
    sleep(10);
    thpool_destroy(thpool);
}