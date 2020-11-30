struct Directional_Light
{
	float strength;
	vec3 direction;
	vec3 color;
	vec3 attenuation;

	bool is_dirty = true; // re-render shadowmap

	Shadow_Map shadow_map;
};

struct Light_Manager
{
	vec3 ambient_light = vec3(0.2);
	Array<Directional_Light> directional_lights;
};

namespace lights
{
	void add_directional_light(Light_Manager& m, const vec3& direction, const vec3& color, const vec3& attenuation, float strength, const Shadow_Map::Config& shadow_map_config);
}

