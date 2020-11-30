namespace
{
	enum Render_Mode : int
	{
		RENDER_VOXELIZED_SCENE,
		RENDER_SCENE,
		RENDER_SHADOW_MAP
	};

	struct Shaders
	{
		Shader_Program model;
		Shader_Program world_pos;
		Shader_Program gbuffer;
		Shader_Program shadowmap;
		Shader_Program shadowmap_visualizer;
		Shader_Program voxelconetracing;
		Shader_Program voxelization;
		Shader_Program voxelization_visualizer;
	};

	struct Renderer
	{
		Render_Mode mode = RENDER_SCENE;
		Camera fps_camera;

		G_Buffer g_buffer;
		Frame_Buffer main_fbo;
		Voxelization voxelization; // see voxel_cone_tracing.h
		Voxelization_Settings voxelization_settings;

		bool visualize_gbuffers = false;
		bool is_first_frame = true;
		bool voxelize_next_frame = true;
		bool render_light_bulbs = false;
	};

	Shaders& get_shaders() {
		static Shaders shaders;
		return shaders;
	}
	Renderer& get_renderer() {
		static Renderer renderer;
		return renderer;
	}

	void voxelize_scene(Scene& scene, GLuint mainFboId, Texture3D& voxel_grid, Voxelization& voxelization_state, Voxelization_Settings& voxelization_settings);
	void render_voxelized_scene(Scene& scene, Camera& camera, GLuint mainFboId, Texture3D& voxel_grid, Voxelization& voxelization_state, Voxelization_Settings& voxelization_settings);
	void render_shadowmaps(Scene& scene, GLuint mainFboId);
	void render_shadowmap_to_screen(Shadow_Map& shadow_map, GLuint mainFboId);
	void render_scene_without_shenanigans(Scene& scene, Camera& camera);
	void render_scene_to_gbuffer(Scene& scene, Camera& camera, GLuint mainFboId, G_Buffer& gb);
	void render_scene_with_voxel_cone_tracing(Scene& scene, Camera& camera, GLuint mainFboId, G_Buffer& gbuf, Texture3D& voxel_grid);
	void upload_camera(GLuint shader_id, Camera& camera);
	void upload_material(GLuint shader_id, Material& material, int texture_location_offset = 0);
	void upload_lights(GLuint shader_id, Light_Manager& light_manager);
	void upload_shadowmap(GLuint shader_id,  Light_Manager& light_manager, int texture_location_offset);
	void upload_voxel_scale(GLuint shader_id, Scene& scene, int current_voxel_resolution);
	void draw_simple_mesh(GLuint shader_id, Mesh& mesh);
	void draw_models_with_materials(GLuint shader_id, Scene& scene, int texture_location_offset = 0);
	void draw_models_with_albedo(GLuint shader_id, Scene& scene, int texture_location_offset);
	void draw_models_without_materials(GLuint shader_id, Scene& scene);

	Texture3D& get_current_voxelgrid();
	void voxel_grid_resolution_changed(int new_resolution_index);
	int  get_current_voxelgrid_resolution();

	void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
}

namespace renderer
{
	void init(GLFWwindow* window)
	{
		LOG("renderer", "initializing");
		Renderer& renderer = get_renderer();

		int window_w, window_h;
		glfwGetWindowSize(window, &window_w, &window_h);

		// load shaders first (early exit if some of them doesn't compile)
		Shaders& shaders = get_shaders();
		shader::init(shaders.model, "shader_model", "../src/shaders/model_vert.glsl", "../src/shaders/model_frag.glsl");
		shader::init(shaders.world_pos, "shader_world_pos", "../src/shaders/world_pos_vert.glsl", "../src/shaders/world_pos_frag.glsl");
		shader::init(shaders.gbuffer, "shader_gbuffer", "../src/shaders/gbuffer_vert.glsl", "../src/shaders/gbuffer_frag.glsl");
		shader::init(shaders.shadowmap, "shader_shadowmap", "../src/shaders/shadowmap_vert.glsl", "../src/shaders/shadowmap_frag.glsl");
		shader::init(shaders.shadowmap_visualizer, "shader_shadowmap_visualizer", "../src/shaders/shadowmap_visualizer_vert.glsl", "../src/shaders/shadowmap_visualizer_frag.glsl");
		shader::init(shaders.voxelconetracing, "shader_voxelconetracing", "../src/shaders/voxelconetracing_vert.glsl", "../src/shaders/voxelconetracing_frag.glsl");
		shader::init(shaders.voxelization, "shader_voxelization", "../src/shaders/voxelization_vert.glsl", "../src/shaders/voxelization_frag.glsl", "../src/shaders/voxelization_geom.glsl");
		shader::init(shaders.voxelization_visualizer, "shader_voxelization_visualizer", "../src/shaders/voxelization_visualizer_vert.glsl", "../src/shaders/voxelization_visualizer_frag.glsl");
		check_gl_error();

		// fbos
		Resolution& resolution = resolution::get();
		framebuffer::init_with_depth(renderer.main_fbo, resolution.internal.x, resolution.internal.y);
		framebuffer::init(renderer.voxelization.vox_front, resolution.internal.x, resolution.internal.y);
		framebuffer::init(renderer.voxelization.vox_back, resolution.internal.x, resolution.internal.y);
		gbuffer::init(renderer.g_buffer, resolution.internal.x, resolution.internal.y);
		check_gl_error();

		// voxel grids
		for (int i = 0; i < TOTAL_VOXELGRID_RESOLUTIONS; i++)
		{
			int dimension = VOXELGRID_RESOLUTIONS[i];
			GLfloat* data = new GLfloat[dimension * dimension * dimension * 4]; // 4 = r,g,b,a (@Cleanup)
			texture3D::fill_corners(data, dimension,dimension,dimension);
			texture3D::init(renderer.voxelization.resolutions[i], data, dimension);
			delete[] data; // uploaded to gpu, no need to keep in app memory (@Cleanup)
		}
		check_gl_error();

		// misc crap
		camera::set_to_ortho(renderer.voxelization.camera);
		camera::set_to_perspective(renderer.fps_camera, resolution.internalAspectRatio);
		check_gl_error();

		glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
		check_gl_error();
	}

	void uninit()
	{
		LOG("renderer", "destroying");
		Renderer& renderer = get_renderer();

		for (int i = 0; i < TOTAL_VOXELGRID_RESOLUTIONS; i++)
			texture3D::uninit(renderer.voxelization.resolutions[i]);

		framebuffer::uninit(renderer.main_fbo);
		framebuffer::uninit(renderer.voxelization.vox_front);
		framebuffer::uninit(renderer.voxelization.vox_back);
		gbuffer::uninit(renderer.g_buffer);
	}

	void render(GLFWwindow* window, Scene& scene, float dt)
	{
		Renderer& renderer = get_renderer();
		Resolution& resolution = resolution::get();

		check_gl_error();
		camera::update(renderer.fps_camera);

		// render to main fbo
		{
			GLuint fboID = renderer.main_fbo.fbo_id;

			static const GLfloat bg_color[] = { 0.75, 0.25, 0.25, 1.0 }; 
			glBindFramebuffer(GL_FRAMEBUFFER, fboID);
			glViewport(0, 0, resolution.internal.x, resolution.internal.y);
			glClearBufferfv(GL_COLOR, 0, bg_color);
			glClear(GL_DEPTH_BUFFER_BIT);

			if (renderer.voxelize_next_frame) {
				renderer.voxelize_next_frame = false;
				render_shadowmaps(scene, fboID);
				voxelize_scene(scene, fboID, get_current_voxelgrid(), renderer.voxelization, renderer.voxelization_settings);
			}

			switch (renderer.mode)
			{
				case RENDER_VOXELIZED_SCENE:
				{
					render_voxelized_scene(scene, renderer.fps_camera, fboID, get_current_voxelgrid(), renderer.voxelization, renderer.voxelization_settings);
				}
				break;

				case RENDER_SCENE:
				{
					check_gl_error();
					render_shadowmaps(scene, fboID);
					render_scene_to_gbuffer(scene, renderer.fps_camera, fboID, renderer.g_buffer);
					render_scene_with_voxel_cone_tracing(scene, renderer.fps_camera, fboID, renderer.g_buffer, get_current_voxelgrid());

					if (renderer.visualize_gbuffers)
						gbuffer::blit_to_screen(renderer.g_buffer, resolution.internal.x, resolution.internal.y);
				}
				break;

				case RENDER_SHADOW_MAP:
				{
					render_shadowmaps(scene, fboID);

					if (array::size(scene.light_manager.directional_lights) > 0)
						render_shadowmap_to_screen(scene.light_manager.directional_lights[0].shadow_map, fboID);
				}
				break;
			}
		}

		// blit main fbo to actual window
		{
			static const GLfloat bg_color[] = { 0.75, 0.75, 0.75, 1.0 };
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, resolution.window.x, resolution.window.y);
			glClearBufferfv(GL_COLOR, 0, bg_color);
			glClear(GL_DEPTH_BUFFER_BIT);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer.main_fbo.fbo_id); 
			glReadBuffer(GL_COLOR_ATTACHMENT0);

			glBlitFramebuffer(
				0, 0, resolution.internal.x, resolution.internal.y,
				resolution.position0.x, resolution.position0.y,
				resolution.position1.x, resolution.position1.y,
				GL_COLOR_BUFFER_BIT, GL_LINEAR);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glReadBuffer(0);
		}

		renderer.is_first_frame = false;
	}

	void render_ui()
	{
		using namespace ImGui;

		Renderer& renderer = get_renderer();
		Scene& scene = scenes::get_current();

		if (TreeNode("Voxelization"))
		{
			if (vct::render_ui(renderer.voxelization_settings)) // shader settings
				renderer.voxelize_next_frame = true;

			Text("voxel grid resolution");
			int resolution_index = renderer.voxelization.current_resolution;
			RadioButton("64",  &resolution_index, 0); ImGui::SameLine();
			RadioButton("128", &resolution_index, 1); ImGui::SameLine();
			RadioButton("256", &resolution_index, 2); ImGui::SameLine();
			RadioButton("512", &resolution_index, 3);
			if (resolution_index != renderer.voxelization.current_resolution) 
				voxel_grid_resolution_changed(resolution_index);

			if (Button("voxelize")) 
				renderer.voxelize_next_frame = true;

			TreePop();
		}

		if (TreeNode("Voxel cone tracing")) {
			vct::render_ui(scene.vct_settings, get_current_voxelgrid_resolution());
			TreePop();
		}

		if (TreeNode("Renderer"))
		{
			Text("mode");
			int mode = renderer.mode;
			RadioButton("render voxelized scene", &mode, Render_Mode::RENDER_VOXELIZED_SCENE);
			RadioButton("render scene with cone tracing", &mode, Render_Mode::RENDER_SCENE);
			RadioButton("render shadow map", &mode, Render_Mode::RENDER_SHADOW_MAP);
			renderer.mode = (Render_Mode) mode;
			Text("");

			Checkbox("visualize g-buffers", &renderer.visualize_gbuffers);
			Checkbox("render light bulbs", &renderer.render_light_bulbs);
			Text("");

			if (TreeNode("camera")) {
				Camera& c = renderer.fps_camera;
				Text("up %.2f,%.2f,%.2f", c.up.x, c.up.y, c.up.z);
				Text("rotation %.2f,%.2f,%.2f", c.direction.x, c.direction.y, c.direction.z);
				Text("position %.2f,%.2f,%.2f", c.position.x, c.position.y, c.position.z);
				TreePop();
			}

			TreePop();
		}
	}

	Camera& get_camera() {
		return get_renderer().fps_camera;
	}
}

namespace
{
	void render_scene_without_shenanigans(Scene& scene, Camera& camera)
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLuint shader_id = shader::activate(get_shaders().model);

		upload_camera(shader_id, camera);
		draw_models_with_materials(shader_id, scene);

		shader::deactivate();
	}

	void render_scene_to_gbuffer(Scene& scene, Camera& camera, GLuint mainFboId, G_Buffer& gb)
	{
		GLuint shader_id = shader::activate(get_shaders().gbuffer);
		gbuffer::activate(gb);

		upload_camera(shader_id, camera);
		draw_models_with_materials(shader_id, scene);

		gbuffer::deactivate(gb);
		shader::deactivate();

		glBindFramebuffer(GL_FRAMEBUFFER, mainFboId);
	}

	void voxelize_scene(Scene& scene, GLuint mainFboId, Texture3D& voxel_grid, Voxelization& voxelization_state, Voxelization_Settings& voxelization_settings)
	{
		LOG("renderer", "voxelizing scene");

		texture3D::clear(voxel_grid, { 0.0f, 0.0f, 0.0f, 0.0f });

		GLuint shader_id = shader::activate(get_shaders().voxelization);
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, voxel_grid.dimensions, voxel_grid.dimensions);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			vct::upload_voxelization_settings(shader_id, voxelization_settings);
			upload_voxel_scale(shader_id, scene, voxel_grid.dimensions);
			upload_camera(shader_id, voxelization_state.camera);
			upload_lights(shader_id, scene.light_manager);

			texture3D::activate(voxel_grid, shader_id, "u_tex_voxelgrid");
			glBindImageTexture(0, voxel_grid.id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
			upload_shadowmap(shader_id, scene.light_manager, 1);

			draw_models_with_albedo(shader_id, scene, 2);
		}
		shader::deactivate();

		texture3D::generate_mipmaps(voxel_grid);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glBindFramebuffer(GL_FRAMEBUFFER, mainFboId);
	}

	void render_voxelized_scene(Scene& scene, Camera& camera, GLuint mainFboId, Texture3D& voxel_grid, Voxelization& voxelization_state, Voxelization_Settings& voxelization_settings)
	{
		// render FBOs
		{
			GLuint shader_id = shader::activate(get_shaders().world_pos);

			glClearColor(0.0, 0.0, 0.0, 1.0);
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			upload_camera(shader_id, camera);

			// render back of cube
			{
				glCullFace(GL_FRONT);
				glBindFramebuffer(GL_FRAMEBUFFER, voxelization_state.vox_back.fbo_id);
				glViewport(0, 0, voxelization_state.vox_back.width, voxelization_state.vox_back.height);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				draw_simple_mesh(shader_id, assets::get_unit_cube());
			}

			// render front of cube
			{
				glCullFace(GL_BACK);
				glBindFramebuffer(GL_FRAMEBUFFER, voxelization_state.vox_front.fbo_id);
				glViewport(0, 0, voxelization_state.vox_front.width, voxelization_state.vox_front.height);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				draw_simple_mesh(shader_id, assets::get_unit_cube());
			}

			shader::deactivate();
		}

		// render 3D texture
		{
			GLuint shader_id = shader::activate(get_shaders().voxelization_visualizer);

			//glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, mainFboId);
			glViewport(0, 0, resolution::get().internal.x, resolution::get().internal.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

			vct::upload_voxelization_settings(shader_id, voxelization_settings);
			upload_camera(shader_id, camera);

			texture3D::activate(voxel_grid, shader_id, "u_tex_voxelgrid", 0);
			framebuffer::activate_color_as_texture(voxelization_state.vox_back, shader_id, "u_tex_cube_back", 1);
			framebuffer::activate_color_as_texture(voxelization_state.vox_front, shader_id, "u_tex_cube_front", 2);
			draw_simple_mesh(shader_id, assets::get_unit_quad());
			texture3D::deactivate();

			shader::deactivate();
		}
	}

	void render_shadowmaps(Scene& scene, GLuint mainFboId)
	{
		check_gl_error();
		glEnable(GL_DEPTH_TEST);

		GLuint shader_id = shader::activate(get_shaders().shadowmap);

		for (Directional_Light& light : scene.light_manager.directional_lights)
		{
			if (light.is_dirty)
			{
				light.is_dirty = false;

				glUniformMatrix4fv(glGetUniformLocation(shader_id, "VP_shadow"), 1, GL_FALSE, glm::value_ptr(light.shadow_map.VP));

				shadowmap::fbo_activate(light.shadow_map);
				draw_models_without_materials(shader_id, scene);
				shadowmap::fbo_deactivate(light.shadow_map);
			}
		}

		shader::deactivate();
		glBindFramebuffer(GL_FRAMEBUFFER, mainFboId);
	}

	void render_shadowmap_to_screen(Shadow_Map& shadow_map, GLuint mainFboId)
	{
		GLuint shader_id = shader::activate(get_shaders().shadowmap_visualizer);

		glBindFramebuffer(GL_FRAMEBUFFER, mainFboId);
		glViewport(0, 0, resolution::get().internal.x, resolution::get().internal.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniform1i(glGetUniformLocation(shader_id, "u_tex_shadowmap"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadow_map.depth_texture_id);

		draw_simple_mesh(shader_id, assets::get_unit_quad());

		shader::deactivate();
	}

	void render_scene_with_voxel_cone_tracing(Scene& scene, Camera& camera, GLuint mainFboId, G_Buffer& gbuf, Texture3D& voxel_grid)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mainFboId);
		glViewport(0, 0, resolution::get().internal.x, resolution::get().internal.y);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		GLuint shader_id = shader::activate(get_shaders().voxelconetracing);
		{
			vct::upload_cone_tracing_settings(shader_id, scene.vct_settings, voxel_grid.dimensions); // voxel_cone_tracing.h
			upload_camera(shader_id, camera);
			upload_voxel_scale(shader_id, scene, voxel_grid.dimensions);
			upload_lights(shader_id, scene.light_manager);

			texture3D::activate(voxel_grid, shader_id, "u_tex_voxelgrid", 0);
			upload_shadowmap(shader_id, scene.light_manager, 1);
			gbuffer::bind_as_textures(gbuf, mainFboId, shader_id, 2);
			draw_simple_mesh(shader_id, assets::get_unit_quad());
			texture3D::deactivate();
		}
		shader::deactivate();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}

	void upload_camera(GLuint shader_id, Camera& camera)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader_id, "VP"), 1, GL_FALSE, glm::value_ptr(camera.VP));
		glUniform3fv(glGetUniformLocation(shader_id, "u_camera_world_position"), 1, glm::value_ptr(camera.position));
	}

	void upload_material(GLuint shader_id, Material& material, int texture_location_offset)
	{
		using glm::value_ptr;
		
		glUniform3fv(glGetUniformLocation(shader_id, "u_material.Ka"), 1, value_ptr(material.Ka));
		glUniform3fv(glGetUniformLocation(shader_id, "u_material.Kd"), 1, value_ptr(material.Kd));
		glUniform3fv(glGetUniformLocation(shader_id, "u_material.Ks"), 1, value_ptr(material.Ks));
		glUniform1f(glGetUniformLocation(shader_id, "u_material.Ns"), material.Ns);
		glUniform3fv(glGetUniformLocation(shader_id, "u_material.Ke"), 1, value_ptr(material.Ke));
		glUniform1f(glGetUniformLocation(shader_id, "u_material.d"), material.d);
		glUniform1f(glGetUniformLocation(shader_id, "u_material.Ni"), material.Ni);
		glUniform3fv(glGetUniformLocation(shader_id, "u_material.Tf"), 1, value_ptr(material.Tf));

		texture::activate(*material.map_Ka,   shader_id, "u_tex_ambient", texture_location_offset + 0);
		texture::activate(*material.map_Kd,   shader_id, "u_tex_diffuse", texture_location_offset + 1);
		texture::activate(*material.map_Ks,   shader_id, "u_tex_specular", texture_location_offset + 2);
		texture::activate(*material.map_Ke,   shader_id, "u_tex_emission", texture_location_offset + 3);
		texture::activate(*material.map_bump, shader_id, "u_tex_bumpmap", texture_location_offset + 4);
	}

	void upload_lights(GLuint shader_id, Light_Manager& light_manager)
	{
		glUniform3fv(glGetUniformLocation(shader_id, "u_ambient_light"), 1, glm::value_ptr(light_manager.ambient_light));

		{
			glUniform1i(glGetUniformLocation(shader_id, "u_total_directional_lights"), array::size(light_manager.directional_lights));

			int index = 0;
			for (Directional_Light& p : light_manager.directional_lights)
			{
				// @Todo @Speed @Cleanup string allocations
				glUniform1f(glGetUniformLocation(shader_id,  ("u_directional_lights[" + std::to_string(index) + "].strength").c_str()), p.strength);
				glUniform3fv(glGetUniformLocation(shader_id, ("u_directional_lights[" + std::to_string(index) + "].direction").c_str()), 1, glm::value_ptr(p.direction));
				glUniform3fv(glGetUniformLocation(shader_id, ("u_directional_lights[" + std::to_string(index) + "].attenuation").c_str()), 1, glm::value_ptr(p.attenuation));
				glUniform3fv(glGetUniformLocation(shader_id, ("u_directional_lights[" + std::to_string(index) + "].color").c_str()), 1, glm::value_ptr(p.color));
				index++;
			}
		}
	}

	void upload_shadowmap(GLuint shader_id, Light_Manager& light_manager, int texture_location_offset) // @Note: only 1 supported atm
	{
		if (array::size(light_manager.directional_lights) > 0) {
			Directional_Light& light = light_manager.directional_lights[0];
			glUniformMatrix4fv(glGetUniformLocation(shader_id, "u_shadowmap_mvp"), 1, GL_FALSE, glm::value_ptr(light.shadow_map.VP_biased));
			shadowmap::texture_activate(light.shadow_map, shader_id, "u_tex_shadowmap", texture_location_offset);
		} else {
			// no shadowmap -- throw in a white texture instead so everything will be visible.
			static mat4 identity_matrix = mat4(1.0f);
			glUniformMatrix4fv(glGetUniformLocation(shader_id, "u_shadowmap_mvp"), 1, GL_FALSE, glm::value_ptr(identity_matrix));
			texture::activate(assets::get_white_texture(), shader_id, "u_tex_shadowmap", texture_location_offset);
		}
	}

	void upload_voxel_scale(GLuint shader_id, Scene& scene, int current_voxel_resolution)
	{
		glUniform3fv(glGetUniformLocation(shader_id, "u_scene_voxel_scale"), 1, glm::value_ptr(scene.voxel_scale));
	}

	void draw_simple_mesh(GLuint shader_id, Mesh& mesh)
	{
		glBindVertexArray(mesh.vao);

		static mat4 im = mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader_id, "M"), 1, GL_FALSE, glm::value_ptr(im));

		for (Sub_Mesh& sub_mesh : mesh.sub_meshes)
			glDrawArrays(GL_TRIANGLES, sub_mesh.index, sub_mesh.length);

		glBindVertexArray(0);
	}

	void draw_models_with_materials(GLuint shader_id, Scene& scene, int texture_location_offset)
	{
		for (Model* model : scene.models) {
			glUniformMatrix4fv(glGetUniformLocation(shader_id, "M"), 1, GL_FALSE, glm::value_ptr(model->transform.mtx));
			glUniformMatrix4fv(glGetUniformLocation(shader_id, "N"), 1, GL_FALSE, glm::value_ptr(model->transform.normal_mtx));

			for (Mesh* mesh : model->meshes) {
				glBindVertexArray(mesh->vao);

				for (Sub_Mesh& sub_mesh : mesh->sub_meshes) { // btw, usually n = 1 here, only a few models have more than 1 material per mesh.
					upload_material(shader_id, assets::get_material(sub_mesh.material_index), texture_location_offset);
					glDrawArrays(GL_TRIANGLES, sub_mesh.index, sub_mesh.length);
				}
			}
		}
	}

	void draw_models_with_albedo(GLuint shader_id, Scene& scene, int texture_location_offset)
	{
		for (Model* model : scene.models) {
			glUniformMatrix4fv(glGetUniformLocation(shader_id, "M"), 1, GL_FALSE, glm::value_ptr(model->transform.mtx));
			glUniformMatrix4fv(glGetUniformLocation(shader_id, "N"), 1, GL_FALSE, glm::value_ptr(model->transform.normal_mtx));

			for (Mesh* mesh : model->meshes) {
				glBindVertexArray(mesh->vao);

				for (Sub_Mesh& sub_mesh : mesh->sub_meshes) {
					Material& material = assets::get_material(sub_mesh.material_index);
					glUniform3fv(glGetUniformLocation(shader_id, "u_material.Ka"), 1, value_ptr(material.Ka));
					glUniform3fv(glGetUniformLocation(shader_id, "u_material.Kd"), 1, value_ptr(material.Kd));
					glUniform3fv(glGetUniformLocation(shader_id, "u_material.Ke"), 1, value_ptr(material.Ke));
					texture::activate(*material.map_Ka, shader_id, "u_tex_ambient", texture_location_offset + 0);
					texture::activate(*material.map_Kd, shader_id, "u_tex_diffuse", texture_location_offset + 1);
					texture::activate(*material.map_Kd, shader_id, "u_tex_emission", texture_location_offset + 2);
					glDrawArrays(GL_TRIANGLES, sub_mesh.index, sub_mesh.length);
				}
			}
		}
	}

	void draw_models_without_materials(GLuint shader_id, Scene& scene)
	{
		for (Model* model : scene.models) {
			glUniformMatrix4fv(glGetUniformLocation(shader_id, "M"), 1, GL_FALSE, glm::value_ptr(model->transform.mtx));
			glUniformMatrix4fv(glGetUniformLocation(shader_id, "N"), 1, GL_FALSE, glm::value_ptr(model->transform.normal_mtx));

			for (Mesh* mesh : model->meshes) {
				glBindVertexArray(mesh->vao);

				for (Sub_Mesh& sub_mesh : mesh->sub_meshes) // @Speed: store the length of the whole vertex buffer so no need to iterate here
					glDrawArrays(GL_TRIANGLES, sub_mesh.index, sub_mesh.length);
			}
		}
	}

	void voxel_grid_resolution_changed(int new_resolution_index) {
		assert(new_resolution_index < TOTAL_VOXELGRID_RESOLUTIONS);

		Renderer& r = get_renderer();
		r.voxelization.current_resolution = new_resolution_index;
		r.voxelize_next_frame = true;

		LOG("renderer", "new voxel resolution: %d", VOXELGRID_RESOLUTIONS[new_resolution_index]);
	}
	int get_current_voxelgrid_resolution() {
		Renderer& r = get_renderer();
		return VOXELGRID_RESOLUTIONS[r.voxelization.current_resolution];
	}
	Texture3D& get_current_voxelgrid() {
		Renderer& r = get_renderer();
		return r.voxelization.resolutions[r.voxelization.current_resolution];
	}

	void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		resolution::window_size_changed(width, height);
	}
}