cmake_minimum_required(VERSION 3.6)

project(vxgi)

set(PATH_ROOT                                                           "${CMAKE_CURRENT_SOURCE_DIR}")
set(PATH_SRC                                                            "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(CMAKE_C_STANDARD                                                    99)
set(CMAKE_C_FLAGS                   "${CMAKE_C_FLAGS}                   ")
set(CMAKE_C_FLAGS_DEBUG             "${CMAKE_C_FLAGS_DEBUG}             -O0 -ggdb -g ")
set(CMAKE_C_FLAGS_RELEASE           "${CMAKE_C_FLAGS_RELEASE}           -O3          ")
set(CMAKE_CXX_STANDARD                                                  20)
set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS}                 -std=c++20 -fno-rtti -fno-exceptions")
set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG}           -std=c++20 -fno-rtti -fno-exceptions -DE_DEBUG   -O0 -ggdb -g")
set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE}         -std=c++20 -fno-rtti -fno-exceptions -DE_RELEASE -O3         ")
set(CMAKE_CXX_FLAGS_RELEASE+ASSERTS "${CMAKE_CXX_FLAGS_RELEASE+ASSERTS} -std=c++20 -fno-rtti -fno-exceptions -DE_RELEASE -O3         ")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY   $<1:${PATH_ROOT}/data>)

include_directories(
	${PATH_ROOT}/
	${PATH_ROOT}/src
	${PATH_ROOT}/lib
	${PATH_ROOT}/lib/imgui
	${PATH_ROOT}/lib/GLFW/include
)

add_definitions(
	-fno-rtti
	-fno-exceptions
	-ferror-limit=10
	-fmessage-length=0
)

add_subdirectory(${PATH_ROOT}/lib/GLFW "glfw")
add_subdirectory(${PATH_ROOT}/lib/GL "glew")
add_subdirectory(${PATH_ROOT}/lib/imgui "imgui")
add_subdirectory(${PATH_ROOT}/lib/lodepng "lodepng")
add_subdirectory(${PATH_ROOT}/lib/tinyobjloader "tinyobjloader")
add_subdirectory(${PATH_ROOT}/lib/stb "stb")

add_compile_definitions(
)

file(GLOB PROJECT_CORE
	${PATH_SRC}/app.cpp
	${PATH_SRC}/app.h
	${PATH_SRC}/assets.cpp
	${PATH_SRC}/assets.h
	${PATH_SRC}/camera.cpp
	${PATH_SRC}/camera.h
	${PATH_SRC}/containers.hpp
	${PATH_SRC}/geometry.h
	${PATH_SRC}/main.cpp
	${PATH_SRC}/opengl.cpp
	${PATH_SRC}/opengl.h
	${PATH_SRC}/renderer.cpp
	${PATH_SRC}/renderer.h
	${PATH_SRC}/scene.cpp
	${PATH_SRC}/scene.h
	${PATH_SRC}/types.h
	${PATH_SRC}/voxel_cone_tracing.cpp
	${PATH_SRC}/voxel_cone_tracing.h
)
source_group(core FILES ${PROJECT_CORE})

set(PROJECT_SRC
	${PROJECT_CORE}
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(opengl glu32 opengl32 gdi32)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(opengl dl GL X11 -pthread)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# WIP - compiles, but doesn't run. metal implementation coming some day
	set(CMAKE_OSX_ARCHITECTURES "arm64")
	add_compile_options(
		-Wall
		-Wno-unused-function
		-Wno-unused-variable
		-Wno-deprecated-volatile # glm
	)
	add_compile_definitions(
		GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
	)
	set(opengl "-framework OpenGL -framework Cocoa -framework AppKit -framework IOKit -framework CoreVideo -framework CoreFoundation")
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SRC})
target_link_libraries(${PROJECT_NAME} ${opengl} glfw glew imgui lodepng tinyobjloader stb)
