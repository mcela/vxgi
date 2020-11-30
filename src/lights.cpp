namespace lights
{
	void add_directional_light(Light_Manager& m, const vec3& direction, const vec3& color, const vec3& attenuation, float strength, const Shadow_Map::Config& shadow_map_config)
	{
		Directional_Light new_light;
		new_light.direction = direction;
		new_light.color = color;
		new_light.attenuation = attenuation;
		new_light.strength = strength;

		shadowmap::init(new_light.shadow_map, shadow_map_config);
		shadowmap::update(new_light.shadow_map, direction);

		array::add(m.directional_lights, new_light);
	}
}
