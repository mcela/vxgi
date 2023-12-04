#include "voxel_cone_tracing.h"

#include "lib/imgui/imgui.h"

namespace vxgi
{
	namespace
	{
		void ApertureSlider(float* output, float min = 1.0f, float max = 179.0f);
		void VoxelDistanceSlider(const char* title, float* output, int voxel_grid_resolution, float min = 1.0f);
	}

	namespace vct
	{
		void upload_voxelization_settings(GLuint shader_id, Voxelization_Settings& settings)
		{
			glUniform1i(glGetUniformLocation(shader_id, "u_settings.use_ambient_light"), settings.use_ambient_light);
			glUniform1i(glGetUniformLocation(shader_id, "u_settings.visualize_mipmap_level"), settings.visualize_mipmap_level);
		}

		void upload_cone_tracing_settings(GLuint shader_id, Cone_Tracing_Shader_Settings& settings, int voxel_grid_resolution)
		{
			auto upload_cone_settings = [shader_id](const std::string& name, Cone_Settings& settings) {
				glUniform1f(glGetUniformLocation(shader_id, (name + ".aperture").c_str()), settings.aperture);
				glUniform1f(glGetUniformLocation(shader_id, (name + ".sampling_factor").c_str()), settings.sampling_factor);
				glUniform1f(glGetUniformLocation(shader_id, (name + ".distance_offset").c_str()), settings.distance_offset);
				glUniform1f(glGetUniformLocation(shader_id, (name + ".max_distance").c_str()), settings.max_distance);
				glUniform1f(glGetUniformLocation(shader_id, (name + ".result_intensity").c_str()), settings.result_intensity);
				glUniform1i(glGetUniformLocation(shader_id, (name + ".is_enabled").c_str()), settings.is_enabled);
			};

			upload_cone_settings("settings.diffuse", settings.diffuse_settings);
			upload_cone_settings("settings.specular", settings.specular_settings);
			upload_cone_settings("settings.softshadows", settings.soft_shadows_settings);
			upload_cone_settings("settings.ao", settings.ao_settings);

			glUniform1f(glGetUniformLocation(shader_id, "settings.direct_light_intensity"), settings.direct_light_intensity);
			glUniform1i(glGetUniformLocation(shader_id, "settings.trace_ao_separately"), settings.trace_ao_separately);
			glUniform1i(glGetUniformLocation(shader_id, "settings.voxel_grid_resolution"), voxel_grid_resolution);
			glUniform1f(glGetUniformLocation(shader_id, "settings.voxel_size"), 1.0f / float(voxel_grid_resolution));
			glUniform1i(glGetUniformLocation(shader_id, "settings.max_mipmap_level"), log2(voxel_grid_resolution));
			glUniform1f(glGetUniformLocation(shader_id, "settings.gamma"), settings.gamma);
			glUniform1f(glGetUniformLocation(shader_id, "settings.hard_shadow_bias"), settings.hard_shadow_bias);
			glUniform1i(glGetUniformLocation(shader_id, "settings.enable_direct_light"), settings.enable_direct_light);
			glUniform1i(glGetUniformLocation(shader_id, "settings.enable_hard_shadows"), settings.enable_hard_shadows);
		}

		float get_aperture(float degrees) {
			return tanf(DEGREES_TO_RADIANS * degrees * 0.5f);
		}

		bool render_ui(Voxelization_Settings& settings)
		{
			using namespace ImGui;
			bool was_clicked = false;

			if (Checkbox("use ambient light", &settings.use_ambient_light)) was_clicked = true;
			if (SliderInt("visualization mipmap level", &settings.visualize_mipmap_level, 0, log2(VOXELGRID_RESOLUTIONS[TOTAL_VOXELGRID_RESOLUTIONS - 1]))) was_clicked = true;

			return was_clicked;
		}

		void render_ui(Cone_Tracing_Shader_Settings& settings, int voxel_grid_resolution)
		{
			using namespace ImGui;

			auto draw_cone_settings = [voxel_grid_resolution](const char* name, Cone_Settings& settings) {
				PushID(name);

				Text("%s", name);
				Checkbox("is_enabled", &settings.is_enabled);
				ApertureSlider(&settings.aperture, 1.0f, 179.0f);
				SliderFloat("sampling_factor", &settings.sampling_factor, 0.1f, 2.0f);
				VoxelDistanceSlider("distance_offset", &settings.distance_offset, voxel_grid_resolution);
				SliderFloat("max_distance", &settings.max_distance, 0.00001f, 2.0f);
				SliderFloat("result_intensity", &settings.result_intensity, 0.0f, 10.0f);
				//SliderInt("direction mode", &diffuseConeDirectionMode, 1, 4);
				//SliderInt("weight mode", &diffuseConeWeightMode, 1, 2);

				PopID();
			};

			draw_cone_settings("Indirect diffuse", settings.diffuse_settings);
			draw_cone_settings("Indirect specular", settings.specular_settings);
			draw_cone_settings("Soft shadows", settings.soft_shadows_settings);

			if (settings.trace_ao_separately) 
				draw_cone_settings("Ambient occlusion", settings.ao_settings);
			else
				Text("Ambient occlusion");
			Checkbox("trace ao separately", &settings.trace_ao_separately);

			PushID("Post processing");
			Text("Post processing");
			Checkbox("enable direct light", &settings.enable_direct_light);
			Checkbox("enable hard shadows", &settings.enable_hard_shadows);
			SliderFloat("direct light intensity", &settings.direct_light_intensity, 0.0f, 2.0f);
			SliderFloat("hard shadow bias", &settings.hard_shadow_bias, 0.0f, 0.01f, "%.6f");
			SliderFloat("gamma", &settings.gamma, 1.0f, 10.0f);
			PopID();
		}
	}

	namespace
	{
		void ApertureSlider(float* output, float min, float max) {
			float apertureDegrees = RADIANS_TO_DEGREES * atan(*output) * 2.0f; // inverse from output

			if (ImGui::SliderFloat("aperture (deg)", &apertureDegrees, min, max))
				*output = vct::get_aperture(apertureDegrees);

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%.2f", *output);
		}

		void VoxelDistanceSlider(const char* title, float* output, int voxel_grid_resolution, float min) {
			float voxelSize = 1.0f / voxel_grid_resolution;
			float voxels = *output * voxel_grid_resolution;

			if (ImGui::SliderFloat(title, &voxels, min, 256.0f))
				*output = voxels * voxelSize;

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%.2f", *output);
		}
	}
}