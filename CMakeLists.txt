cmake_minimum_required(VERSION 2.6)

project(xpipe)

include(CTest)

set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

if(UNIX)
    # TODO: use properties to set the compiler standard
    add_definitions("-Wall -Wextra -pedantic -Wno-long-long -std=c++11")
endif()

add_subdirectory("xpipe")
add_subdirectory("sample")
