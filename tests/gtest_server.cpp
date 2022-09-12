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
    uint32_t get_port(char * port);
    uint8_t get_threads(char * thread);
}

class ServerTestValidPorts : public ::testing::TestWithParam<std::tuple<std::string, bool>>{};

TEST_P(ServerTestValidPorts, TestValidPorts)
{
    auto [port_str, expect_failure] = GetParam();

    uint32_t port = get_port((char *)port_str.c_str());
    if (expect_failure)
    {
        EXPECT_EQ(port, 0);
    }
    else
    {
        EXPECT_TRUE(port >= MIN_PORT && port <= MAX_PORT);
    }
}

INSTANTIATE_TEST_SUITE_P(
    PortTest,
    ServerTestValidPorts,
    ::testing::Values(
        std::make_tuple("1", false),
        std::make_tuple("60000", false),
        std::make_tuple("65535", false),
        std::make_tuple("65535", false),
        std::make_tuple("65536", true),
        std::make_tuple("0", true)
        ));


class ServerTestValidThreads : public ::testing::TestWithParam<std::tuple<std::string, bool>>{};

TEST_P(ServerTestValidThreads, TestValidPorts)
{
    auto [thread_str, expect_failure] = GetParam();

    uint32_t threads = get_threads((char *)thread_str.c_str());
    if (expect_failure)
    {
        EXPECT_EQ(threads, 0);
    }
    else
    {
        EXPECT_TRUE(threads >= MIN_THREADS && threads <= MAX_THREADS) << "Threads str: " << thread_str << " Converted: " << threads;
    }
}

INSTANTIATE_TEST_SUITE_P(
    PortTest,
    ServerTestValidThreads,
    ::testing::Values(
        std::make_tuple("1", false),
        std::make_tuple("240", false),
        std::make_tuple("255", false),
        std::make_tuple("256", true),
        std::make_tuple("0", true)
    ));
class ServerCmdTester : public ::testing::TestWithParam<std::tuple<std::vector<std::string>, bool>>{};

// Parameter test handles signed testing
TEST_P(ServerCmdTester, TestingServerArguments)
{
    // Reset getopt value for testing
    optind = 0;

    auto [str_vector, expect_failure] = GetParam();
    int arg_count = (int)str_vector.size();

    // Turn the string vector into a array of char array
    char ** argv = new char * [str_vector.size()];
    for(size_t i = 0; i < str_vector.size(); i++)
    {
        argv[i] = new char[str_vector[i].size() + 1];
        strcpy(argv[i], str_vector[i].c_str());
    }

    // Call the test function
    args_t * args = parse_args(arg_count, argv);
    if (expect_failure)
    {
        EXPECT_EQ(args, nullptr);
    }
    else
    {
        EXPECT_NE(args, nullptr);
    }


    // Free the array of char arrays
    for(size_t i = 0; i < str_vector.size(); i++)
    {
        delete [] argv[i];
    }
    delete [] argv;
    free_args(args);
}

INSTANTIATE_TEST_SUITE_P(
    AdditionTests,
    ServerCmdTester,
    ::testing::Values(
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "31337"}, false),
        std::make_tuple(std::vector<std::string>{__FILE__, "-h"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "1"}, false),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "65535"}, false),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "65536"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "0"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "4000", "-n", "8"}, false),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "4000", "-n", "256"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "4000", "-n", "0"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-n", "8"}, false),
        std::make_tuple(std::vector<std::string>{__FILE__, "-n", "256"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-n", "0"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-n", "0", "extra_arg"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "4000", "-n", "8", "extra_arg"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p", "4000", "-n", "8", "-n", "9000"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-p"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "extra_arg"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-w"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__, "-w", "10", "-p", "10"}, true),
        std::make_tuple(std::vector<std::string>{__FILE__}, false)
    ));