#include <gtest/gtest.h>
#include <thread_pool.h>

void work_func_one(void * arg)
{
    printf("Got in here\n");
    std::atomic_int64_t * val = (std::atomic_int64_t *)arg;
    printf("Value is %ld\n", std::atomic_load(val));
    std::atomic_fetch_add(val, 10);
    printf("Value is %ld\n", std::atomic_load(val));
}

class ThreadPoolTextFixture : public ::testing::Test
{
 public:
    thpool_t * thpool;
 protected:
    void SetUp() override
    {
        this->thpool = thpool_init(4);
    }
    void TearDown() override
    {
        thpool_destroy(this->thpool);
    }
};

TEST_F(ThreadPoolTextFixture, TestInit)
{
    EXPECT_NE(this->thpool, nullptr);
}

TEST_F(ThreadPoolTextFixture, TestVarUpdating)
{
    std::atomic_int64_t * val = (std::atomic_int64_t *)calloc(1, sizeof(std::atomic_int64_t));
    thpool_enqueue_job(this->thpool, work_func_one, val);
    EXPECT_EQ(std::atomic_load(val), 10);
    free(val);
}



