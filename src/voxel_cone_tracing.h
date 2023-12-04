#pragma once

#include "camera.h"
#include "opengl.h"

namespace vxgi
{
	const int TOTAL_VOXELGRID_RESOLUTIONS = 4;
	const int VOXELGRID_RESOLUTIONS[TOTAL_VOXELGRID_RESOLUTIONS] = { 64, 128, 256, 512 };
	const int DEFAULT_VOXELGRID_RESOLUTION_INDEX = 2;

	struct Voxelization
	{
		// for visualizing voxelized scene
		Frame_Buffer vox_front;
		Frame_Buffer vox_back; 

		Camera camera; // orthographic projection

		Texture3D resolutions[TOTAL_VOXELGRID_RESOLUTIONS]; // 64, 128, 256, 512
		int current_resolution = DEFAULT_VOXELGRID_RESOLUTION_INDEX; // index to array above ^
	};

	struct Voxelization_Settings // for shaders
	{
		bool use_ambient_light = true;
		int visualize_mipmap_level = 0;
	};

	struct Cone_Settings
	{
		float aperture; // aperture = tan(degrees * 0.5)
		float sampling_factor; // 0.0...1.0
		float distance_offset; // voxel_size * distance_in_voxels
		float max_distance; // in NDC space -1.0...1.0
		float result_intensity;
		bool is_enabled;
	};

	struct Cone_Tracing_Shader_Settings
	{
		Cone_Settings diffuse_settings      = { 0.577f, 0.119f, 0.081f, 2.0f, 1.0f, true };
		Cone_Settings specular_settings     = { 0.027f, 0.146f, 0.190f, 2.0f, 1.0f, true };
		Cone_Settings soft_shadows_settings = { 0.017f, 0.200f, 0.120f, 2.0f, 1.0f, true };
		Cone_Settings ao_settings           = { 0.577f, 1.000f, 0.500f, 1.0f, 1.0f, true };
		bool trace_ao_separately = false; // otherwise use diffuse cone opacity

		float gamma = 2.2f;
		float hard_shadow_bias = 0.005f;
		float direct_light_intensity = 1.0f;
		bool enable_direct_light = true;
		bool enable_hard_shadows = false; // shadow mapped
	};

	namespace vct
	{
		void upload_voxelization_settings(GLuint shader_id, Voxelization_Settings& settings);
		void upload_cone_tracing_settings(GLuint shader_id, Cone_Tracing_Shader_Settings& settings, int voxel_grid_resolution);

		float get_aperture(float degrees);

		bool render_ui(Voxelization_Settings& settings);
		void render_ui(Cone_Tracing_Shader_Settings& settings, int voxel_grid_resolution);
	}
}