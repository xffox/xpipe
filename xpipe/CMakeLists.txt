cmake_minimum_required(VERSION 2.8)

set(XPIPE_NAME "xpipe")

find_package(Threads REQUIRED)

set(XPIPE_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(XPIPE_INCLUDES ${XPIPE_INCLUDES} PARENT_SCOPE)
set(XPIPE_LIBS ${XPIPE_NAME})
set(XPIPE_LIBS ${XPIPE_LIBS} PARENT_SCOPE)

include_directories(${XPIPE_INCLUDES})

file(GLOB XPIPE_SRCS "src/*.cpp")

add_library(${XPIPE_NAME} STATIC ${XPIPE_SRCS})
target_link_libraries(${XPIPE_NAME} Threads::Threads)

add_subdirectory(test)
