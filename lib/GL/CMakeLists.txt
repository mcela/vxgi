cmake_minimum_required(VERSION 3.10)

project(glew)

set(CMAKE_C_STANDARD                                                    99)
set(CMAKE_C_FLAGS                   "${CMAKE_C_FLAGS}                   ")
set(CMAKE_C_FLAGS_DEBUG             "${CMAKE_C_FLAGS_DEBUG}             -O0 -ggdb -g ")
set(CMAKE_C_FLAGS_RELEASE           "${CMAKE_C_FLAGS_RELEASE}           -O3          ")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/
)

set(SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/glew.c
	${CMAKE_CURRENT_SOURCE_DIR}/glew.h
)

add_library(${PROJECT_NAME} ${SOURCE_FILES})