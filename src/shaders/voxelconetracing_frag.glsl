#version 450 core

#define PI 3.14159265f
#define MAX_DIRECTIONAL_LIGHTS 4

// See http://simonstechblog.blogspot.com/2013/01/implementing-voxel-cone-tracing.html
const int TOTAL_DIFFUSE_CONES = 6;
const vec3 DIFFUSE_CONE_DIRECTIONS[TOTAL_DIFFUSE_CONES] = { vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.5f, 0.866025f), vec3(0.823639f, 0.5f, 0.267617f), vec3(0.509037f, 0.5f, -0.7006629f), vec3(-0.50937f, 0.5f, -0.7006629f), vec3(-0.823639f, 0.5f, 0.267617f) };
const float DIFFUSE_CONE_WEIGHTS[TOTAL_DIFFUSE_CONES] = { PI / 4.0f, 3.0f * PI / 20.0f, 3.0f * PI / 20.0f, 3.0f * PI / 20.0f,  3.0f * PI / 20.0f, 3.0f * PI / 20.0f };

struct Directional_Light
{
	float strength;
	vec3 direction;
	vec3 color;
	vec3 attenuation;
};

struct Cone_Settings
{
	float aperture;
	float sampling_factor;
	float distance_offset;
	float max_distance;
	float result_intensity;
	int is_enabled;
};

struct Settings
{
	Cone_Settings diffuse;
	Cone_Settings specular;
	Cone_Settings softshadows;
	Cone_Settings ao;
	int trace_ao_separately;
	float gamma;
	float hard_shadow_bias;
	float voxel_size;
	float direct_light_intensity;
	int voxel_grid_resolution;
	int max_mipmap_level;
	int enable_direct_light;
	int enable_hard_shadows;
};

uniform Settings settings;
uniform	vec3 u_ambient_light;
uniform int u_total_directional_lights;
uniform Directional_Light u_directional_lights[MAX_DIRECTIONAL_LIGHTS];
uniform vec3 u_camera_world_position;
uniform vec3 u_scene_voxel_scale;
uniform mat4 u_shadowmap_mvp;

uniform sampler3D u_tex_voxelgrid; 
uniform sampler2DShadow u_tex_shadowmap;
uniform sampler2D g_world_pos;
uniform sampler2D g_normal;
uniform sampler2D g_bump;
uniform sampler2D g_albedo;
uniform sampler2D g_specular;
//uniform sampler2D g_emission;

in vec2 f_tex_coords;
layout(location = 0) out vec4 o_color;

vec3 f_voxel_pos = vec3(0.0f); // -1.0 ... 1.0
vec3 f_world_pos = vec3(0.0f);
vec3 f_normal = vec3(0.0f);
vec3 f_bump = vec3(0.0f);
vec4 f_albedo = vec4(0.0f);
vec4 f_specular = vec4(0.0f); // a = shininess
//vec3 f_emission = vec3(0.0f);
vec4 f_shadow_coord = vec4(0.0f);
float f_visibility = 1.0f; // calculated from shadow map

float attenuate(float dist, float strength, vec3 attenuation) { 
	return strength / (attenuation.x + attenuation.y * dist + attenuation.z * dist * dist);
}

bool is_inside_clipspace(const vec3 p, float e) {
	return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e;
}

//
// CONE TRACE FUNCTION
// note: aperture = tan(radians * 0.5)
//
vec4 trace_cone(const vec3 start_clip_pos, vec3 direction, float aperture, float distance_offset, float distance_max, float sampling_factor)
{
	aperture = max(0.1f, aperture); // inf loop if 0
	direction = normalize(direction);
	float distance = distance_offset; // avoid self-collision
	vec3 accumulated_color = vec3(0.0f);
	float accumulated_occlusion = 0.0f;
	
	while (distance <= distance_max && accumulated_occlusion < 1.0f)
	{
		vec3 cone_clip_pos = start_clip_pos + (direction * distance);
		vec3 cone_voxelgrid_pos = 0.5f * cone_clip_pos + vec3(0.5f); // from clipspace -1.0...1.0 to texcoords 0.0...1.0

		float diameter = 2.0f * aperture * distance; 
		float mipmap_level = log2(diameter * settings.voxel_grid_resolution);
		vec4 voxel_sample = textureLod(u_tex_voxelgrid, cone_voxelgrid_pos, min(mipmap_level, settings.max_mipmap_level));

		// front to back composition
		accumulated_color += (1.0f - accumulated_occlusion) * voxel_sample.rgb; 
		accumulated_occlusion += (1.0f - accumulated_occlusion) * voxel_sample.a; 

		distance += diameter * sampling_factor;
	}

	accumulated_occlusion = min(accumulated_occlusion, 1.0f);
	return vec4(accumulated_color, accumulated_occlusion);
}

float trace_shadow_cone(vec3 from, vec3 direction, float distance)
{
	vec4 s = trace_cone(from, direction, settings.softshadows.aperture, settings.softshadows.distance_offset, settings.softshadows.max_distance * distance, settings.softshadows.sampling_factor);
	return 1.0f - s.a;
}

vec4 calc_indirect_specular()
{
	vec3 viewDirection = normalize(f_world_pos - u_camera_world_position);
	vec3 coneDirection = normalize(reflect(viewDirection, f_normal));
  	vec4 specularIntensity = vec4(1.0f);

	float aperture = settings.specular.aperture;

	vec3 start_clip_pos = f_voxel_pos + (f_normal * settings.specular.distance_offset);
	
	vec4 specular = trace_cone(start_clip_pos, coneDirection, aperture, settings.specular.distance_offset, settings.specular.max_distance, settings.specular.sampling_factor);
	specular.rgb *= f_specular.rgb;

	return specular;
}

vec4 calc_indirect_diffuse()
{
	vec4 accumulated_color = vec4(0.0f);

	// rotate cone around the normal
	vec3 guide = vec3(0.0f, 1.0f, 0.0f);
	if (abs(dot(f_normal, guide)) == 1.0f)
	  guide = vec3(0.0f, 0.0f, 1.0f);

	// find a tangent and a bitangent
	vec3 right = normalize(guide - dot(f_normal, guide) * f_normal);
	vec3 up = cross(right, f_normal);

	for (int i = 0; i < TOTAL_DIFFUSE_CONES; i++)
	{
		vec3 coneDirection = f_normal;
		coneDirection += DIFFUSE_CONE_DIRECTIONS[i].x * right + DIFFUSE_CONE_DIRECTIONS[i].z * up;
		coneDirection = normalize(coneDirection);

		vec3 start_clip_pos = f_voxel_pos + (f_normal * settings.diffuse.distance_offset);
		accumulated_color += trace_cone(start_clip_pos, coneDirection, settings.diffuse.aperture, settings.diffuse.distance_offset, settings.diffuse.max_distance, settings.diffuse.sampling_factor) * DIFFUSE_CONE_WEIGHTS[i];
	}

	return accumulated_color;
}

float calc_ambient_occlusion() // this is also calculated during diffuse tracing, but we can do it separately with different settings too
{
	// @Todo @Cleanup @Cutnpaste (from calc_indirect_diffuse())

	vec4 accumulated_color = vec4(0.0f);

	vec3 guide = vec3(0.0f, 1.0f, 0.0f);
	if (abs(dot(f_normal, guide)) == 1.0f)
		guide = vec3(0.0f, 0.0f, 1.0f);

	vec3 right = normalize(guide - dot(f_normal, guide) * f_normal);
	vec3 up = cross(right, f_normal);

	for (int i = 0; i < TOTAL_DIFFUSE_CONES; i++)
	{
		vec3 coneDirection = f_normal;
		coneDirection += DIFFUSE_CONE_DIRECTIONS[i].x * right + DIFFUSE_CONE_DIRECTIONS[i].z * up;
		coneDirection = normalize(coneDirection);

		vec3 start_clip_pos = f_voxel_pos + (f_normal * settings.ao.distance_offset);
		accumulated_color += trace_cone(start_clip_pos, coneDirection, settings.ao.aperture, settings.ao.distance_offset, settings.ao.max_distance, settings.ao.sampling_factor) * DIFFUSE_CONE_WEIGHTS[i];
	}

	return accumulated_color.a;
}

#if 0 // cook-torrance
vec3 BRDF(vec3 light_direction, float light_distance, vec3 light_color, float strength, vec3 attenuation) 
{
	float attenuation_factor = attenuate(light_distance, strength, attenuation);

	float mean = 0.7; // mean value of microfacet distribution
	float scale = 0.2; // constant factor C

	vec3 N = f_bump;
	vec3 L = light_direction - f_world_pos; // to light
	vec3 V = u_camera_world_position - f_world_pos; // to eye
	vec3 H = normalize(L + V); // half way 
	float n_h = dot(N,H);
	float n_v = dot(N,V);
	float v_h = dot(V,H);
	float n_l = dot(N,L);

	vec3 diffuse = f_albedo.rgb * max(n_l, 0);
	diffuse *= attenuation_factor;

	float fresnel = pow(1.0f + v_h, 4.0f);
	float delta = acos(n_h);
	float exponent = -pow((delta / mean), 2.0f);
	float microfacets = scale * exp(exponent);
	float term1 = 2 * n_h * n_v / v_h;
	float term2 = 2 * n_h * n_l / v_h;
	float selfshadow = min(1.0f, min(term1, term2));

	vec3 specular = f_specular.rgb * fresnel * microfacets * selfshadow / n_v;
	specular *= attenuation_factor;

	return light_color * (diffuse + specular);
}
#else // blinn-phong
vec3 BRDF(vec3 light_direction, float light_distance, vec3 light_color, float strength, vec3 attenuation)
{
	float attenuationFactor = attenuate(light_distance, strength, attenuation);

	// diffuse
	float diffuseFactor = max(dot(f_normal, light_direction), 0.0f);
	vec3 diffuse = light_color * diffuseFactor * attenuationFactor;

	vec3 toLight = light_direction - f_world_pos;
	vec3 toEye = u_camera_world_position - f_world_pos;
	vec3 halfwayUnit = normalize(toLight + toEye);
	float shininess = f_specular.a;
	float cosRefAngle = clamp(dot(f_normal, halfwayUnit), 0.0, 1.0);
	cosRefAngle = pow(cosRefAngle, shininess);
	vec3 specular = f_specular.rgb * light_color * cosRefAngle * attenuationFactor;

	return (diffuse + specular);
}
#endif

vec4 calc_direct_light()
{
	vec3 totalColor = vec3(0.0f);

	for (int i=0; i < u_total_directional_lights; i++)
	{
		Directional_Light light = u_directional_lights[i];

		vec3 light_direction = light.direction;
		float light_distance = length(light_direction);
		light_direction = normalize(light_direction);

		float visibility = 1.0f; 
		if (settings.softshadows.is_enabled == 1) {
			vec3 start_clip_pos = f_voxel_pos + (f_normal * settings.softshadows.distance_offset);
			visibility = max(0.0f, trace_shadow_cone(start_clip_pos, light_direction, 2.0f));
		}

		totalColor += visibility * BRDF(light_direction, light_distance, light.color, light.strength, light.attenuation);
	}

	return vec4(totalColor, 1.0f);
}

float calc_visibility()
{
	return texture(u_tex_shadowmap, vec3(f_shadow_coord.xy, (f_shadow_coord.z - settings.hard_shadow_bias) / f_shadow_coord.w));
}

void main()
{
	f_world_pos = texture(g_world_pos, f_tex_coords).xyz;
	f_voxel_pos = (f_world_pos * u_scene_voxel_scale);
	f_normal = normalize(texture(g_normal, f_tex_coords).xyz);
	f_bump = normalize(texture(g_bump, f_tex_coords).xyz);
	f_albedo = texture(g_albedo, f_tex_coords);
	f_specular = texture(g_specular, f_tex_coords);
	//f_emission = texture(g_emission, f_tex_coords).rgb;

	if (settings.enable_hard_shadows == 1) {
		f_shadow_coord = u_shadowmap_mvp * vec4(f_world_pos, 1.0f);
		f_visibility = calc_visibility();
	}

	vec4 direct_diffuse_color = vec4(0.0f);
	vec4 indirect_specular_color = vec4(0.0f);
	vec4 indirect_diffuse_color = vec4(0.0f);
	vec4 indirect_light = vec4(0.0f, 0.0f, 0.0f, 1.0f); // alpha component == ambient occlusion

	// @Todo @Cleanup this code, got a bit messy because of the UI options available.

	bool only_render_ao = (
		settings.enable_direct_light == 0 &&
		settings.diffuse.is_enabled == 0 &&
		settings.specular.is_enabled == 0);

	if (only_render_ao)
	{
		if (settings.trace_ao_separately == 1) {
			indirect_light.a = clamp(1.0f - calc_ambient_occlusion(), 0.0f, 1.0f);
		} else {
			indirect_light = calc_indirect_diffuse();
			indirect_light.a = clamp(1.0f - indirect_light.a, 0.0f, 1.0f);
			indirect_light *= f_albedo;
		}

		indirect_light.rgb = vec3(1.0f);
		o_color = vec4(indirect_light.a,indirect_light.a,indirect_light.a, 1.0f);
	}
	else
	{
		if (settings.enable_direct_light == 1)
			direct_diffuse_color = f_albedo * calc_direct_light();

		float ao = 0.0f;

		if (settings.diffuse.is_enabled == 1) {
			indirect_diffuse_color = calc_indirect_diffuse();
			ao = indirect_diffuse_color.a;
			indirect_diffuse_color = f_albedo * settings.diffuse.result_intensity * indirect_diffuse_color;
		}

		if (settings.specular.is_enabled == 1)
			indirect_specular_color = f_albedo * settings.specular.result_intensity * calc_indirect_specular();

		indirect_light = indirect_specular_color + indirect_diffuse_color;
		indirect_light.a = f_albedo.a * 1.0f;

		if (settings.ao.is_enabled == 1) {
			if (settings.trace_ao_separately == 1 || settings.diffuse.is_enabled == 0) 
				indirect_light.a = clamp(1.0f - calc_ambient_occlusion(), 0.0f, 1.0f);
			else
				indirect_light.a = clamp(1.0f - ao, 0.0f, 1.0f);
		}

		vec4 ambient_light = f_albedo * vec4(u_ambient_light, 1.0f) * indirect_light.a;
		vec3 total_light = ambient_light.rgb + (f_visibility * settings.direct_light_intensity * direct_diffuse_color.rgb) + indirect_light.rgb;

		total_light = pow(total_light, vec3(1.0f / settings.gamma));
		o_color = vec4(total_light, 1.0f);
	}
}