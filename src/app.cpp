namespace
{
	struct App_Config
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

	Application& get_app() {
		static Application app;
		return app;
	}

	bool create_window(const App_Config& config);
	void destroy_window();
	void GLFW_error_callback(int error, const char* description);
}

namespace application
{
	bool init(int argc, const char* argv[], const vec2& internal_render_resolution)
	{
		LOG("app", "initializing");
		Application& app = get_app();

		App_Config default_config;
		if (!create_window(default_config))
			return false;

		resolution::set(default_config.window_resolution, internal_render_resolution);
		resolution::scale_with_black_bars();

		renderer::init(app.window);
		flycamera::attach(app.camera_controls, &renderer::get_camera());

		if (app.input_state == Input_State::FPS_CAMERA)
			glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		assets::init();
		if (argc == 2)
			scenes::init(argv[1]);
		else
			scenes::init();

		return true;
	}

	void deinit()
	{
		LOG("app", "uninitializing");
		scenes::uninit();
		assets::uninit();
		renderer::uninit();
		destroy_window();
	}

	void run()
	{
		LOG("app", "starting loop");
		Application& app = get_app();

		bool is_exit_queued = false;
		bool was_f1_pressed = false;

		while (!glfwWindowShouldClose(app.window) && !is_exit_queued)
		{
			float dt = 1.0f / 60.0f; // TODO

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			if (glfwGetKey(app.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
				is_exit_queued = true;
			if (glfwGetKey(app.window, GLFW_KEY_F1) == GLFW_PRESS) 
				was_f1_pressed = true;
			if (glfwGetKey(app.window, GLFW_KEY_F1) == GLFW_RELEASE && was_f1_pressed) { 
				was_f1_pressed = false;
				app.input_state = (app.input_state == Input_State::FPS_CAMERA) ? Input_State::UI_INTERACTION : Input_State::FPS_CAMERA;

				if (app.input_state == Input_State::FPS_CAMERA)
					glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				else
					glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			switch (app.input_state) {
				case Input_State::FPS_CAMERA:
					flycamera::update(app.camera_controls, app.window, dt);
					break;
				case Input_State::UI_INTERACTION:
					render_ui();
					break;
			};

			renderer::render(app.window, scenes::get_current(), dt);

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(app.window);
			glfwPollEvents();
		}

		LOG("app", "ending loop");
	}

	std::string read_file(const char* filepath)
	{
		std::ifstream file(filepath);
		std::string result((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		return result;
	}
}

namespace resolution
{
	Resolution& get()
	{
		static Resolution resolution;
		return resolution;
	}
	void set(vec2 windowResolution, vec2 internalRenderResolution)
	{
		Resolution& resolution = resolution::get();

		resolution.window = windowResolution;
		resolution.internal = internalRenderResolution;
		resolution.windowAspectRatio = resolution.window.x / resolution.window.y;
		resolution.internalAspectRatio = resolution.internal.x / resolution.internal.y;
		resolution.position0 = { 0.0f, 0.0f };
		resolution.position1 = { 0.0f, 0.0f };

		LOG("resolution", "updated!");
		LOG("resolution", "window  : %.0fx%.0f (%f)", resolution.window.x,resolution.window.y,resolution.windowAspectRatio);
		LOG("resolution", "internal: %.0fx%.0f (%f)", resolution.internal.x,resolution.internal.y,resolution.internalAspectRatio);
	}
	void scale_with_black_bars()
	{
		// TODO this is buggy :D

		Resolution& resolution = resolution::get();

		bool doResolutionsMatch = (resolution.internal.x == resolution.window.x && resolution.internal.y == resolution.window.y);
		if (doResolutionsMatch) {
			resolution.position0 = { 0.0f, 0.0f };
			resolution.position1 = resolution.internal;
			return;
		}

		// set the viewport depending on the screen resolution
		float currentWidth = resolution.window.x;
		float currentHeight = resolution.window.y;

		float targetAspectRatio = resolution.internal.x / resolution.internal.y;

		// figure out the largest area that fits in this resolution at the desired aspect ratio
		float width = currentWidth;
		float height = width / targetAspectRatio + 0.5f;

		if (height > currentHeight) {
			// it doesn't fit our height, we must switch to pillarbox then
			height = currentHeight;
			width = height * targetAspectRatio + 0.5f;
		}

		// set up the new viewport centered in the backbuffer
		resolution.position0.x = 0.5f * (currentWidth - width);
		resolution.position0.y = 0.5f * (currentHeight - height);
		resolution.position1.x = width;
		resolution.position1.y = height;

		LOG("resolution", "render from %.0fx%.0f %.0fx%.0f", resolution.position0.x,resolution.position0.y, resolution.position1.x,resolution.position1.y);
	}
	void window_size_changed(int width, int height)
	{
		Resolution& resolution = resolution::get();
		resolution.window.x = width;
		resolution.window.y = height;
		resolution.windowAspectRatio = resolution.window.x / resolution.window.y;
		resolution::scale_with_black_bars();
	}
}

namespace
{
	bool create_window(const App_Config& config)
	{
		LOG("app", "initializing opengl context");
		Application& app = get_app();

		// init GLFW
		{
			if (!glfwInit()) {
				LOG("app", "error in GLFW initialization");
				return false;
			} else {
				LOG("app", "glfw inited (version %s)", glfwGetVersionString());
			}
		}

		// init OpenGL profile
		{
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // GLFW_OPENGL_COMPAT_PROFILE); 
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.gl_major_version);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.gl_minor_version);
			glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE); // note: disabled for windowed mode windows
			glfwWindowHint(GLFW_SAMPLES, config.msaa_samples);
		}

		// init window
		{
			switch (config.window_mode)
			{
				case App_Config::Window_Mode::WINDOWED_FULLSCREEN:
				{
					LOG("app", "creating windowed fullscreen");

					GLFWmonitor *monitor = glfwGetPrimaryMonitor();
					const GLFWvidmode *mode = glfwGetVideoMode(monitor);

					glfwWindowHint(GLFW_RED_BITS, mode->redBits);
					glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
					glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
					glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

					app.window = glfwCreateWindow(mode->width, mode->height, config.window_title, monitor, NULL);
				}
				break;

				case App_Config::Window_Mode::WINDOWED:
				{
					app.window = glfwCreateWindow(config.window_resolution.x, config.window_resolution.y, config.window_title, NULL, NULL);
				}
				break;
			}

			if (!app.window) {
				LOG("app", "error in window creation");
				glfwTerminate();
				return false;
			} else {
				LOG("app", "window created");
			}

			glfwMakeContextCurrent(app.window);
			glfwSwapInterval(config.vsync_mode);
		}

		// init GLEW
		{
			glewExperimental = GL_TRUE; 
			GLenum err = glewInit();

			if (err != GLEW_OK) {
				LOG("app", "error in GLEW initialization: %s", glewGetErrorString(err));
				glfwTerminate();
			} else {
				LOG("app", "glew inited (version %s)", glewGetString(GLEW_VERSION));
			}
		}

		// init OpenGL settings
		{
			//glEnable(GL_TEXTURE_3D);
			glEnable(GL_MULTISAMPLE); 
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			check_gl_error();

			LOG("app", "OpenGL version %s, GLSL version %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
		}

		// init ImGui
		{
			LOG("app", "ImGui version %s", IMGUI_VERSION);
			ImGui::CreateContext();
			ImGui_ImplGlfw_InitForOpenGL(app.window, true);
			ImGui_ImplOpenGL3_Init("#version 450 core");
			check_gl_error();
		}

		{
			glfwSetErrorCallback(GLFW_error_callback);
		}

		LOG("app", "opengl initialization ready");
		return true;
	}

	void destroy_window()
	{
		Application& app = get_app();

		LOG("app", "terminating imgui");
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		LOG("app", "terminating glfw");
		glfwDestroyWindow(app.window);
		glfwTerminate();
	}

	void GLFW_error_callback(int error, const char* description)
	{
		LOG("app", "glfw error %d: %s", error, description);
	}
}
