cmake_minimum_required(VERSION 3.10)

project(tinyobjloader)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/
)

set(PATH_HERE ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES
	${PATH_HERE}/tiny_obj_loader.cc
	${PATH_HERE}/tiny_obj_loader.h
)

add_library(${PROJECT_NAME} ${SOURCE_FILES})