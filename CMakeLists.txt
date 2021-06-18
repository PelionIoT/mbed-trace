cmake_minimum_required (VERSION 3.11)
SET(CMAKE_SYSTEM_NAME Generic)

project(mbedTrace)

option(enable_coverage_data "Enable Coverage data" OFF)

include(repositories.cmake)

add_library(mbedTrace STATIC)
target_sources(mbedTrace PRIVATE source/mbed_trace.c)

target_include_directories(mbedTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(mbedTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mbed-trace/)

target_link_libraries(mbedTrace nanostack-libservice)


# Tests after this line
enable_testing()

if (enable_coverage_data)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

add_executable(trace_test
    source/mbed_trace.c
    test/stubs/ip6tos_stub.c
    test/Test.cpp
)
target_include_directories(trace_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mbed-trace)
target_include_directories(trace_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(trace_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/test/stubs)

target_link_libraries(
    trace_test
    gtest_main
    nanostack-libservice
)

# GTest framework requires C++ version 11
set_target_properties(trace_test
PROPERTIES
    CXX_STANDARD 11
)

include(GoogleTest)
gtest_discover_tests(trace_test)

if (enable_coverage_data)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html")

    add_test(NAME ls_cov
        COMMAND ${BASH} -c "gcovr -r ../ . -e ${CMAKE_CURRENT_SOURCE_DIR}/test --html --html-details -o html/example-html-details.html"
    )
endif ()
