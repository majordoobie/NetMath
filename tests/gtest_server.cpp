#include <gtest/gtest.h>
#include <server.h>

extern "C"
{
    typedef struct args_t
    {
        uint32_t port;
        uint8_t threads;
    } args_t;
    args_t * parse_args(int argc, char ** argv);
    void free_args(args_t * args);
}

class ServerCmdTester : public ::testing::TestWithParam<std::tuple<std::vector<std::string>, bool>>{};

// Parameter test handles signed testing
TEST_P(ServerCmdTester, TestingServerArguments)
{
    auto [str_vector, expect_failure] = GetParam();
    int arg_count = (int)str_vector.size();

    char ** arr = new char*[str_vector.size()];
    for(size_t i = 0; i < str_vector.size(); i++)
    {
        arr[i] = new char[str_vector[i].size() + 1];
        strcpy(arr[i], str_vector[i].c_str());
    }

    args_t * args = parse_args(arg_count, arr);
    if (expect_failure)
    {
        EXPECT_EQ(args, nullptr);
    }
    else
    {
        EXPECT_NE(args, nullptr);
    }


    for(size_t i = 0; i < str_vector.size(); i++)
    {
        delete [] arr[i];
    }
    delete [] arr;

    free_args(args);
}


INSTANTIATE_TEST_SUITE_P(
    AdditionTests,
    ServerCmdTester,
    ::testing::Values(
        std::make_tuple(std::vector<std::string>{__FILE__}, false)
    ));


TEST(One, One)
{
    args_t * args = parse_args(1, (char **)"1");
    EXPECT_NE(nullptr, args);
    free_args(args);

}