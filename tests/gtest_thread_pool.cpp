#include <gtest/gtest.h>
#include <thread_pool.h>

void work_func_one(void * arg)
{
    std::atomic_int64_t * val = (std::atomic_int64_t *)arg;
    std::atomic_fetch_add(val, 10);
    sleep(1);
}

void work_func_two(void * arg)
{
    printf("Got callback\n");
    char * name = (char *)arg;
    printf("[||]: %s\n", name);
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
    thpool_enqueue_job(this->thpool, work_func_one, val);
    thpool_wait(this->thpool);
    EXPECT_EQ(std::atomic_load(val), 20);
    free(val);
}

TEST_F(ThreadPoolTextFixture, TestMultiUpdating)
{
    std::atomic_int64_t * val = (std::atomic_int64_t *)calloc(1, sizeof(std::atomic_int64_t));

    int i = 0;
    while (i < 100)
    {
        thpool_enqueue_job(this->thpool, work_func_one, val);
        i = i + 10;
    }

    thpool_wait(this->thpool);
    EXPECT_EQ(std::atomic_load(val), i);
    free(val);
}



