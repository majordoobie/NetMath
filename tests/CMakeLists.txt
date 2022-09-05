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

