cmake_minimum_required(VERSION 3.16)

enable_testing()
set(CTEST_RERUN_FAILED ON)
set(CTEST_OUTPUT_ON_FAILURE ON)

project(
    JQRCalcProjects
    LANGUAGES "C"
    DESCRIPTION "JQR Example Projects"
)

if (DEBUG EQUAL "1")
    message("DEBUGGING VERSION")
    add_compile_definitions(DEBUG)
else()
    message("RELEASE VERSION")
endif()



include_directories(0_Common/include)

# repeat for other projects
add_subdirectory(0_Common)
add_subdirectory(1_SimpleCalc)


# repeat for other projects
add_test(
    NAME TestSimpleCalc
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND bash ${CMAKE_SOURCE_DIR}/local_tester.sh 1_SimpleCalc
)


