namespace
{
	Scene_Manager& get_scene_manager() {
		static Scene_Manager mgr;
		return mgr;
	}

	static const Scene_Config suzanne_config =
	{
		.name                      = "suzanne",
		.obj_file_path             = "monkey.obj",
		.res_file_path             = "",

		.scale                     = 1.0f,

		.sun_direction             = vec3(0.3f, 0.3f, 1.0f),
		.sun_color                 = vec3(1.0f, 1.0f, 1.0f),
		.sun_attenuation           = vec3(1.0f, 1.0f, 1.0f),
		.sun_strength              = 1.0f,
		.shadow_map_config         = { 1024, 1.5, -1.0f, 3.5f },

		.vct_config =
		{
			.diffuse_settings      = { vct::get_aperture(60.00f), 0.272f, 32.0f, 2.0f, 1.0f, true },
			.specular_settings     = { vct::get_aperture(5.000f), 1.000f,  2.0f, 2.0f, 1.0f, true },
			.soft_shadows_settings = { vct::get_aperture(1.676f), 1.865f, 3.17f, 2.0f, 1.0f, true },
			.ao_settings           = { vct::get_aperture(60.00f), 0.594f, 10.0f, 0.6f, 1.0f, true }
		}
	};

	static const Scene_Config cornell_config =
	{
		.name                      = "cornell",
		.obj_file_path             = "cornell.obj",
		.res_file_path             = "",

		.scale                     = 1.0f,

		.sun_direction             = vec3(0.3f, 0.3f, 1.0f),
		.sun_color                 = vec3(1.0f, 1.0f, 1.0f),
		.sun_attenuation           = vec3(1.0f, 1.0f, 1.0f),
		.sun_strength              = 1.0f,
		.shadow_map_config         = { 2048, 1.5, -1.0f, 3.5f },

		.vct_config =
		{
			.diffuse_settings      = { vct::get_aperture(60.00f), 0.272f, 32.0f, 2.0f, 1.0f, true },
			.specular_settings     = { vct::get_aperture(5.000f), 1.000f,  2.0f, 2.0f, 1.0f, true },
			.soft_shadows_settings = { vct::get_aperture(1.676f), 1.865f, 3.17f, 2.0f, 1.0f, true },
			.ao_settings           = { vct::get_aperture(60.00f), 1.000f, 10.0f, 0.6f, 1.0f, true }
		}
	};

	static const Scene_Config sponza_config =
	{
		.name                      = "Crytek Sponza",
		.obj_file_path             = "sponza.obj",
		.res_file_path             = "sponzatextures/",

		.scale                     = 0.01f,

		.sun_direction             = vec3(0.061f, 0.242f, 0.0f),
		.sun_color                 = vec3(1.0f, 1.0f, 1.0f),
		.sun_attenuation           = vec3(1.0f, 1.0f, 1.0f),
		.sun_strength              = 1.0f,
		.shadow_map_config         = { 4096, 8.0f, -20.0f, 25.0f },

		.vct_config =
		{
			.diffuse_settings      = { vct::get_aperture(60.00f), 0.100f,  3.9f, 2.0f, 1.0f, true },
			.specular_settings     = { vct::get_aperture(0.700f), 1.000f,  1.0f, 2.0f, 1.0f, true },
			.soft_shadows_settings = { vct::get_aperture(1.293f), 0.359f, 10.7f, 2.0f, 1.0f, false },
			.ao_settings           = { vct::get_aperture(60.00f), 0.594f, 10.0f, 0.5f, 1.0f, true }
		}
	};
}

namespace scene
{
	void init(Scene& scene, const Scene_Config& config)
	{
		scene.name = config.name;

		assets::load(scene.models, scene.bounding_box, config.obj_file_path, config.res_file_path);
		lights::add_directional_light(scene.light_manager, config.sun_direction, config.sun_color, config.sun_attenuation, config.sun_strength, config.shadow_map_config);

		float gridsize = (float) VOXELGRID_RESOLUTIONS[DEFAULT_VOXELGRID_RESOLUTION_INDEX];
		scene.vct_settings = config.vct_config;
		scene.vct_settings.diffuse_settings.distance_offset /= gridsize;
		scene.vct_settings.specular_settings.distance_offset /= gridsize;
		scene.vct_settings.soft_shadows_settings.distance_offset /= gridsize;
		scene.vct_settings.ao_settings.distance_offset /= gridsize;

		scene.scale = config.scale;
		scene.bounding_box.min_point *= config.scale;
		scene.bounding_box.max_point *= config.scale;
		boundingbox::update(scene.bounding_box);

		// make sure scene center point is at 0,0,0 so that all of it fits into the voxel grid
		for (Model* model : scene.models) {
			model->transform.scale = vec3(config.scale);
			model->transform.position = -scene.bounding_box.center;
			transform::update(model->transform);
		}

		// 2.0f = NDC is [-1, 1] so abs(1 - -1) = 2.0f
		const float offset = 0.1f; // small offset so that a vertex at bounds (1,1,1) will be voxelized aswell
		scene.voxel_scale = vec3(
			(2.0f - offset) / fabs(scene.bounding_box.max_point.x - scene.bounding_box.min_point.x),
			(2.0f - offset) / fabs(scene.bounding_box.max_point.y - scene.bounding_box.min_point.y),
			(2.0f - offset) / fabs(scene.bounding_box.max_point.z - scene.bounding_box.min_point.z));
	}
	void uninit(Scene& scene)
	{
		// TODO
	}
}

namespace scenes
{
	void init()
	{
		init("cornell");
	}
	void init(const char* str)
	{
		if      (strcmp("suzanne", str) == 0) scene::init(scenes::get_current(), suzanne_config);
		else if (strcmp("cornell", str) == 0) scene::init(scenes::get_current(), cornell_config);
		else if (strcmp("sponza",  str) == 0) scene::init(scenes::get_current(), sponza_config);
		else {
			LOG("scene", "i dunno wat '%s' is :(", str);
			scene::init(scenes::get_current(), cornell_config);
		}
	}
	void uninit()
	{
		scene::uninit(scenes::get_current());
	}
	Scene& get_current()
	{
		return get_scene_manager().current;
	}
}