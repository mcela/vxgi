enum Input_State
{
	FPS_CAMERA,
	UI_INTERACTION
};

struct Application
{
	GLFWwindow* window = NULL;
	Input_State input_state = FPS_CAMERA;
	Fly_Camera_Controls camera_controls;
};

struct Application_Config
{
	enum Window_Mode
	{
		WINDOWED_FULLSCREEN,
		WINDOWED,
		//FULLSCREEN
	};

	const char* window_title = "VXGI 2020";
	Window_Mode window_mode = WINDOWED;

	int gl_major_version = 4;
	int gl_minor_version = 5;
	int msaa_samples = 0;
	int vsync_mode = 1; // -1, 0, 1, https://www.glfw.org/docs/3.3/window_guide.html#buffer_swap

	vec2 window_resolution = vec2(1920, 1080); // used if window_mode == WINDOWED
};

struct Resolution
{
	vec2 window;
	vec2 internal;

	float windowAspectRatio = 1.0f;
	float internalAspectRatio = 1.0f;

	vec2 position0;
	vec2 position1;
};

namespace application
{
	bool init(int argc, const char* argv[], const vec2& internal_render_resolution = vec2(1920, 1080));
	void deinit();
	void run();

	std::string read_file(const char* filepath);
}

namespace resolution
{
	Resolution& get();
	void        set(vec2 windowResolution, vec2 internalRenderResolution);
	void        scale_with_black_bars();
	void        window_size_changed(int width, int height);
}
