cmake_minimum_required(VERSION 3.10)

project(imgui)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/
)

set(PATH_HERE ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES
	${PATH_HERE}/imgui.cpp
	${PATH_HERE}/imgui.h
	${PATH_HERE}/examples/imgui_impl_opengl3.h
	${PATH_HERE}/examples/imgui_impl_opengl3.cpp
	${PATH_HERE}/examples/imgui_impl_glfw.h
	${PATH_HERE}/examples/imgui_impl_glfw.cpp
	${PATH_HERE}/imgui_demo.cpp
	${PATH_HERE}/imgui_draw.cpp
	${PATH_HERE}/imgui_widgets.cpp
)

add_library(${PROJECT_NAME} ${SOURCE_FILES})