#pragma once

#include "geometry.h"
#include "voxel_cone_tracing.h"

namespace vxgi
{
	struct Directional_Light
	{
		float strength;
		vec3 direction;
		vec3 color;
		vec3 attenuation;

		bool is_dirty = true; // re-render shadowmap

		Shadow_Map shadow_map;
	};

	struct Scene_Lights
	{
		vec3 ambient_light = vec3(0.2);
		Array<Directional_Light> directional_lights;
	};

	struct Scene
	{
		const char*                    name = "";
		float                          scale = 1.0f;
		vec3                           voxel_scale;
		Bounding_Box                   bounding_box;
		Cone_Tracing_Shader_Settings   vct_settings;
		Scene_Lights                   lights;
		Array<Model*>                  models; // note: model memory owned by Assets.
	};

	struct Scene_Config
	{
		const char* name = "";
		const char* obj_file_path = "";
		const char* res_file_path = "";

		float scale = 1.0f;

		vec3  sun_direction;
		vec3  sun_color;
		vec3  sun_attenuation;
		float sun_strength;
		Shadow_Map::Config shadow_map_config;

		Cone_Tracing_Shader_Settings vct_config;
	};

	struct Scene_Manager
	{
		Scene current;
	};

	namespace scene
	{
		void init(Scene&, const Scene_Config&);
		void uninit(Scene&);

		void add_directional_light(Scene&, const vec3& direction, const vec3& color, const vec3& attenuation, float strength, const Shadow_Map::Config&);
	}

	namespace scenes
	{
		void init();
		void init(const char* selectedSceneName);
		void uninit();
		Scene& get_current();
	}
}