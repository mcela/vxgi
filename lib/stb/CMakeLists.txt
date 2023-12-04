cmake_minimum_required(VERSION 3.10)

project(stb)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/
)

set(PATH_HERE ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES
	${PATH_HERE}/stb_ds.cpp
	${PATH_HERE}/stb_ds.h
)

add_library(${PROJECT_NAME} ${SOURCE_FILES})