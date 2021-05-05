cmake_minimum_required (VERSION 3.11)
SET(CMAKE_SYSTEM_NAME Generic)

project(mbedTrace)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mbed-trace/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../nanostack-libservice/mbed-client-libservice/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../nanostack-libservice/)

set (MBED_TRACE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/mbed_trace.c)


add_library(mbedTrace "${MBED_TRACE_SRC}" "")

