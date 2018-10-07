cmake_minimum_required(VERSION 2.8)

set(SAMPLE_NAME "sample")

include_directories(${XPIPE_INCLUDES} ${CSV_INCLUDES})

file(GLOB SAMPLE_SRCS "*.cpp")

add_executable(${SAMPLE_NAME} ${SAMPLE_SRCS})
target_link_libraries(${SAMPLE_NAME} ${XPIPE_LIBS})
