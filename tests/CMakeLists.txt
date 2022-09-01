include(BuildUtils)

#
# Calculation testing target
#
add_executable(
        gtest_calculation
        gtest_calculation.cpp
)

target_link_libraries(
        gtest_calculation
        PUBLIC
        calculation
)
GTest_add_target(gtest_calculation)

#
# Header Parser testing target
#
add_executable(
        gtest_header_parser
        gtest_header_parser.cpp
)
target_link_libraries(
        gtest_header_parser
        PUBLIC
        header_parser
)

GTest_add_target(gtest_header_parser)
