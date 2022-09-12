include(BuildUtils)

#
# Test the actual calculations between operands and test the parsing of the
# headers coming in
#
add_executable(
        gtest_parser
        gtest_calculation.cpp
        gtest_header_parser.cpp
)

target_link_libraries(
        gtest_parser
        PUBLIC
        calculation
        header_parser
)
GTest_add_target(gtest_parser)

#
# Test the thread_pool library
#
add_executable(
        gtest_thread_pool
        gtest_thread_pool.cpp
)
target_link_libraries(
        gtest_thread_pool
        PUBLIC
        thread_pool
)
GTest_add_target(gtest_thread_pool)

#
# Test the server portion of the project
#
add_executable(
        gtest_server
        gtest_server.cpp
)
target_link_libraries(
        gtest_server
        PUBLIC
        server_backend
)
GTest_add_target(gtest_server)
