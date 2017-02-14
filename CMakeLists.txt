INCLUDE(CMakeForceCompiler)

cmake_minimum_required (VERSION 2.8)
SET(CMAKE_SYSTEM_NAME Generic)

project(mbedTrace)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../nanostack-libservice/mbed-client-libservice/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../nanostack-libservice/)


FILE(GLOB NANO_STACK_LIB_SERVICE_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"

	)

CREATE_LIBRARY(mbedTrace "${NANO_STACK_LIB_SERVICE_SRC}" "")

