#include <string> // std::string
#include <assert.h> // assert
#include <fstream> // ifstream, read file
#include <iostream> // log
#include <cmath> // sin, cos

#define GLEW_STATIC
#include <GL/glew.h>
#ifdef _WIN64
#include <GL/glew.c>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <imgui-1.75/imgui.h>
#include <imgui-1.75/examples/imgui_impl_glfw.h>
#include <imgui-1.75/examples/imgui_impl_opengl3.h>
#include <imgui-1.75/examples/imgui_impl_glfw.cpp>
#include <imgui-1.75/examples/imgui_impl_opengl3.cpp>

#include "typedefs.h"
#include "containers.h"
#include "opengl_stuff.h"
#include "geometry.h"
#include "camera.h"
#include "voxel_cone_tracing.h"
#include "lights.h"
#include "scene.h"
#include "assets.h"
#include "renderer.h"
#include "opengl_renderer.h"
#include "app.h"

#include "containers.cpp"
#include "lights.cpp"
#include "scene.cpp"
#include "ui.cpp"
#include "camera.cpp"
#include "opengl_stuff.cpp"
#include "opengl_renderer.cpp"
#include "voxel_cone_tracing.cpp"
#include "app.cpp"
#include "assets.cpp"

int main(int argc, const char* argv[])
{
	LOG("main", "starting application");

	if (application::init(argc, argv, { 1920, 1080 })) {
		application::run();
	}
	application::deinit();

	LOG("main", "exiting application");
	return 0;
}