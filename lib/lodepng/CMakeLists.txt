cmake_minimum_required(VERSION 3.10)

project(lodepng)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/
)

set(PATH_HERE ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES
	${PATH_HERE}/lodepng.cpp
	${PATH_HERE}/lodepng.h
)

add_library(${PROJECT_NAME} ${SOURCE_FILES})