#pragma once

#include "types.h"
#include "renderer.h"
#include "camera.h"
#include "opengl.h"
#include "voxel_cone_tracing.h"

namespace vxgi
{
	enum RENDERER_MODE : u32
	{
		RENDERER_MODE_SCENE,
		RENDERER_MODE_SCENE_VOXELIZED,
		RENDERER_MODE_SCENE_SHADOW_MAP
	};

	struct Renderer_Shaders
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
		RENDERER_MODE mode = RENDERER_MODE_SCENE;
		Camera fps_camera;

		Renderer_Shaders shaders;

		G_Buffer g_buffer;
		Frame_Buffer main_fbo;
		Voxelization voxelization; // see voxel_cone_tracing.h
		Voxelization_Settings voxelization_settings;

		bool visualize_gbuffers = false;
		bool is_first_frame = true;
		bool voxelize_next_frame = true;
		bool render_light_bulbs = false;
	};

	namespace renderer
	{
		void init(GLFWwindow*);
		void uninit();

		void render(GLFWwindow*, Scene&, float dt);
		void render_ui();

		void voxelize_scene(Scene&, GLuint mainFboId, Texture3D& voxel_grid, Voxelization& voxelization_state, Voxelization_Settings& voxelization_settings);
		void render_voxelized_scene(Scene&, Camera& camera, GLuint mainFboId, Texture3D& voxel_grid, Voxelization& voxelization_state, Voxelization_Settings& voxelization_settings);
		void render_shadowmaps(Scene&, GLuint mainFboId);
		void render_shadowmap_to_screen(Shadow_Map& shadow_map, GLuint mainFboId);
		void render_scene_without_shenanigans(Scene&, Camera& camera);
		void render_scene_to_gbuffer(Scene&, Camera& camera, GLuint mainFboId, G_Buffer& gb);
		void render_scene_with_voxel_cone_tracing(Scene&, Camera& camera, GLuint mainFboId, G_Buffer& gbuf, Texture3D& voxel_grid);
		void upload_camera(GLuint shader_id, Camera& camera);
		void upload_material(GLuint shader_id, Material& material, int texture_location_offset = 0);
		void upload_lights(GLuint shader_id, Scene_Lights&);
		void upload_shadowmap(GLuint shader_id,  Scene_Lights&, int texture_location_offset);
		void upload_voxel_scale(GLuint shader_id, Scene&, int current_voxel_resolution);
		void draw_simple_mesh(GLuint shader_id, Mesh& mesh);
		void draw_models_with_materials(GLuint shader_id, Scene&, int texture_location_offset = 0);
		void draw_models_with_albedo(GLuint shader_id, Scene&, int texture_location_offset);
		void draw_models_without_materials(GLuint shader_id, Scene&);

		Camera& get_camera();
		Texture3D& get_current_voxelgrid();
		void voxel_grid_resolution_changed(int new_resolution_index);
		int get_current_voxelgrid_resolution();

		void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
	}
}