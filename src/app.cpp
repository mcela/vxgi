#include "app.h"

#include <GLFW/glfw3.h>
#include "lib/imgui/imgui.h"
#include "lib/imgui/examples/imgui_impl_glfw.h"
#include "lib/imgui/examples/imgui_impl_opengl3.h"

#include "assets.h"
#include "containers.hpp"
#include "renderer.h"
#include "scene.h"

namespace vxgi
{
	namespace
	{
		Application& get_app() {
			static Application app;
			return app;
		}

		bool create_window(const Application_Config& config);
		void destroy_window();
		void GLFW_error_callback(int error, const char* description);
		void render_ui();
	}

	namespace application
	{
		bool init(int argc, const char* argv[], const vec2& internal_render_resolution)
		{
			LOG("app", "initializing");
			Application& app = get_app();

			Application_Config default_config;
			if (!create_window(default_config))
				return false;

			resolution_set(default_config.window_size, internal_render_resolution);
			resolution_scale_with_black_bars();

			renderer::init(app.window);
			flycamera::attach(app.camera_controls, &renderer::get_camera());

			if (app.input_state == APPLICATION_INPUT_FPS)
				glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			assets::init();
			if (argc == 2)
				scenes::init(argv[1]);
			else
				scenes::init();

			return true;
		}
		void uninit()
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
				float dt = 1.0f / 60.0f;

				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				if (glfwGetKey(app.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
					is_exit_queued = true;
				}
				if (glfwGetKey(app.window, GLFW_KEY_F1) == GLFW_PRESS) {
					was_f1_pressed = true;
				}
				if (glfwGetKey(app.window, GLFW_KEY_F1) == GLFW_RELEASE && was_f1_pressed) { 
					was_f1_pressed = false;
					app.input_state = (app.input_state == APPLICATION_INPUT_FPS) ? APPLICATION_INPUT_UI : APPLICATION_INPUT_FPS;

					if (app.input_state == APPLICATION_INPUT_FPS) {
						glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					} else {
						glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					}
				}

				switch (app.input_state) {
					case APPLICATION_INPUT_FPS:
						flycamera::update(app.camera_controls, app.window, dt);
						break;
					case APPLICATION_INPUT_UI:
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

		Application_Resolution& resolution_get()
		{
			return get_app().resolution;
		}
		void resolution_set(vec2 windowResolution, vec2 internalRenderResolution)
		{
			Application_Resolution& resolution = resolution_get();

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
		void resolution_scale_with_black_bars()
		{
			// TODO quick buggy hack :D

			Application_Resolution& resolution = resolution_get();

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
		void resolution_window_size_changed(int width, int height)
		{
			Application_Resolution& resolution = resolution_get();
			resolution.window.x = width;
			resolution.window.y = height;
			resolution.windowAspectRatio = resolution.window.x / resolution.window.y;
			resolution_scale_with_black_bars();
		}

		std::string read_file(const char* filepath)
		{
			std::ifstream file(filepath);
			std::string result((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();
			return result;
		}
	}

	namespace
	{
		bool create_window(const Application_Config& config)
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
					case APPLICATION_WINDOW_WINDOWED:
					{
						app.window = glfwCreateWindow(config.window_size.x, config.window_size.y, config.window_title, NULL, NULL);
					}
					break;

					case APPLICATION_WINDOW_WINDOWED_FULLSCREEN:
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

		void render_ui()
		{
			using namespace ImGui;

			Scene& scene = scenes::get_current();

			Begin("debug");
			{
				renderer::render_ui();

				if (TreeNode("Lights"))
				{
					Scene_Lights& lights = scene.lights;

					SliderFloat3("ambient light", (float*) &lights.ambient_light, 0.0f, 1.0f);

					if (TreeNode("Directional lights"))
					{
						for (Directional_Light& p : lights.directional_lights)
						{
							PushID(&p);

							SliderFloat("strength", &p.strength, 0.0f, 10.0f);
							if (SliderFloat3("direction", (float*) &p.direction, -1.0f, 1.0f)) p.is_dirty = true;
							SliderFloat3("color", (float*) &p.color, 0.0f, 1.0f);
							SliderFloat3("attenuation", (float*) &p.attenuation, 0.0f, 1.0f);

							if (TreeNode("shadow map")) {
								if (SliderFloat("ortho size", &p.shadow_map.config.ortho,         1.0f, 100.0f)) p.is_dirty = true;
								if (SliderFloat("near plane", &p.shadow_map.config.near_plane, -100.0f, 100.0f)) p.is_dirty = true;
								if (SliderFloat("far plane",  &p.shadow_map.config.far_plane,     0.0f, 100.0f)) p.is_dirty = true;
								TreePop();
							}

							if (p.is_dirty) {
								shadowmap::update(p.shadow_map, p.direction);
							}

							PopID();
						}
						
						TreePop();
					}

					TreePop();
				}
			}
			End();
		}

		void GLFW_error_callback(int error, const char* description)
		{
			LOG("app", "glfw error %d: %s", error, description);
		}
	}
}