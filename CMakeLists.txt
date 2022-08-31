cmake_minimum_required(VERSION 3.20)
project(JG_NetCalc C CXX)

# Uncomment this line if you need some serious debugging of the make process
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Add the cmake modules directory to call functions in them
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Create the variable for where all the tests will go in the build tree
# this will make it easier to execute all the test binaries
set(TEST_BUILD_PATH ${CMAKE_BINARY_DIR}/test_bin)

# Include the deps
include(deps)

add_subdirectory(src/calculation)

# If debug is enabled make sure to include CTest at the root. This will allow
# the ctest config to be placed at the root of the build directory
IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CTest)
    add_subdirectory(tests)
ENDIF()
