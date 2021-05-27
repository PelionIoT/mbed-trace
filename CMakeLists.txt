cmake_minimum_required (VERSION 3.11)
SET(CMAKE_SYSTEM_NAME Generic)

project(mbedTrace)

set (MBED_TRACE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/mbed_trace.c)


add_library(mbedTrace "${MBED_TRACE_SRC}" "")

target_include_directories(mbedTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(mbedTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mbed-trace/)
