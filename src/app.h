#pragma once

#include "types.h"
#include "camera.h"

namespace vxgi
{
	enum APPLICATION_INPUT_MODE : u32
	{
		APPLICATION_INPUT_FPS,
		APPLICATION_INPUT_UI
	};

	enum APPLICATION_WINDOW_MODE : u32
	{
		//APPLICATION_WINDOW_FULLSCREEN,
		APPLICATION_WINDOW_WINDOWED,
		APPLICATION_WINDOW_WINDOWED_FULLSCREEN,
	};

	struct Application_Config
	{
		const char* window_title = "VXGI 2020";
		APPLICATION_WINDOW_MODE window_mode = APPLICATION_WINDOW_WINDOWED;
		vec2 window_size = vec2(1920, 1080);

		int gl_major_version = 4;
		int gl_minor_version = 5;
		int msaa_samples = 0;
		int vsync_mode = 1; // -1, 0, 1, https://www.glfw.org/docs/3.3/window_guide.html#buffer_swap
	};

	struct Application_Resolution
	{
		vec2 window;
		vec2 internal;

		float windowAspectRatio = 1.0f;
		float internalAspectRatio = 1.0f;

		vec2 position0;
		vec2 position1;
	};

	struct Application
	{
		GLFWwindow* window = nullptr;
		Application_Resolution resolution;
		Camera_Controls_Fly camera_controls;
		APPLICATION_INPUT_MODE input_state = APPLICATION_INPUT_FPS;
	};

	namespace application
	{
		bool init(int argc, const char* argv[], const vec2& internal_render_resolution = vec2(1920, 1080));
		void uninit();

		void run();

		Application_Resolution& resolution_get();
		void resolution_set(vec2 windowResolution, vec2 internalRenderResolution);
		void resolution_scale_with_black_bars();
		void resolution_window_size_changed(int width, int height);

		std::string read_file(const char* filepath);
	}
}
