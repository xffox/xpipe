cmake_minimum_required(VERSION 2.8)

include(FindPkgConfig)

include_directories(${PIPELINE_INCLUDES})

pkg_search_module(CPPUNIT cppunit)
if(CPPUNIT_FOUND)
    set(PIPELINE_TEST_NAME pipeline_tests)
    file(GLOB PIPELINE_TEST_SRCS "*.cpp")
    add_executable(${PIPELINE_TEST_NAME} ${PIPELINE_TEST_SRCS})
    target_link_libraries(${PIPELINE_TEST_NAME} ${XPIPE_LIBS}
        ${CPPUNIT_LIBRARIES})

    add_test(NAME PipelineTests COMMAND ${PIPELINE_TEST_NAME})
else()
    message(WARNING "cppunit not found - not unit tests")
endif()
