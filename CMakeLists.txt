cmake_minimum_required (VERSION 3.11)
SET(CMAKE_SYSTEM_NAME Generic)

project(mbedTrace)

add_library(mbedTrace STATIC)
target_sources(mbedTrace PRIVATE source/mbed_trace.c)

target_include_directories(mbedTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(mbedTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mbed-trace/)

add_dependencies(mbedTrace nanostack-libservice)
target_link_libraries(mbedTrace nanostack-libservice)