struct Scene
{
	const char*                    name = "";
	float                          scale = 1.0f;
	vec3                           voxel_scale;
	Bounding_Box                   bounding_box;
	Cone_Tracing_Shader_Settings   vct_settings;
	Light_Manager                  light_manager;
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
	void init(Scene& scene, const Scene_Config& config);
	void uninit(Scene& scene);
}

namespace scenes
{
	void init();
	void init(const char* selectedSceneName);
	void uninit();
	Scene& get_current();
}
