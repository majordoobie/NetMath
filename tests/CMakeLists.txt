add_executable(
        gtest_calculation
        gtest_calculation.cpp
)

target_link_libraries(
        gtest_calculation
        PUBLIC
        calculation
)
include(BuildUtils)
GTest_add_target(gtest_calculation)