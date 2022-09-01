add_executable(
        gtest_cal_and_parser
        gtest_calculation.cpp
        gtest_header_parser.cpp
)

target_link_libraries(
        gtest_cal_and_parser
        PUBLIC
        calculation
        header_parser
)
include(BuildUtils)
GTest_add_target(gtest_cal_and_parser)
