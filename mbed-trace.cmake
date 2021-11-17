#################################################################################
## Copyright 2021 Pelion.
##
## SPDX-License-Identifier: Apache-2.0
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
#################################################################################

SET(CMAKE_SYSTEM_NAME Generic)

option(enable_coverage_data "Enable Coverage data" OFF)

include(repositories.cmake)

add_library(mbedTrace STATIC)
add_library(mbedTraceInterface INTERFACE)
target_include_directories(mbedTraceInterface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(mbedTraceInterface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mbed-trace/)

target_sources(mbedTrace PRIVATE source/mbed_trace.c)

target_include_directories(mbedTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(mbedTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mbed-trace/)

target_link_libraries(mbedTrace nanostack-libservice)

if (test_all OR ${CMAKE_PROJECT_NAME} STREQUAL "mbedTrace")
    # Tests after this line
    enable_testing()

    if (enable_coverage_data)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
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

    if (enable_coverage_data AND ${CMAKE_PROJECT_NAME} STREQUAL "mbedTrace")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html")

        add_test(NAME trace_cov WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${BASH} -c "gcovr -r . -e ${CMAKE_CURRENT_SOURCE_DIR}/build -e '.*test.*' --html --html-details -o build/html/example-html-details.html"
        )
    endif ()
endif()
