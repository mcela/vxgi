#version 450 core

struct Voxelization_Settings
{
	int use_ambient_light;
	int visualize_mipmap_level;
};

uniform Voxelization_Settings u_settings;
uniform sampler3D u_tex_voxelgrid;
uniform sampler2D u_tex_cube_back;
uniform sampler2D u_tex_cube_front;
uniform vec3 u_camera_world_position;

in vec2 f_tex_coords;
in vec3 f_world_pos;

out vec4 o_color;

bool is_inside_voxelgrid(const vec3 p) {
	return abs(p.x) < 1.1f && abs(p.y) < 1.1f && abs(p.z) < 1.1f;
}

void main()
{
	vec4 accumulated_color = vec4(0,0,0,0);
	vec3 ray_origin = is_inside_voxelgrid(u_camera_world_position) ? u_camera_world_position : texture(u_tex_cube_front, f_tex_coords).xyz;
	vec3 ray_end = texture(u_tex_cube_back, f_tex_coords).xyz;
	vec3 ray_direction = normalize(ray_end - ray_origin);

	const float ray_step_size = 0.003f;
	int total_samples = int(length(ray_end - ray_origin) / ray_step_size);

	for (int i=0; i < total_samples; i++)
	{
		vec3 sample_location = (ray_origin + ray_direction * ray_step_size * i);
		vec4 texSample = textureLod(u_tex_voxelgrid, (sample_location + vec3(1.0f)) * 0.5f, u_settings.visualize_mipmap_level);	

		if (texSample.a > 0) {
			texSample.rgb /= texSample.a;
			accumulated_color.rgb = accumulated_color.rgb + (1.0f - accumulated_color.a) * texSample.a * texSample.rgb;
			accumulated_color.a   = accumulated_color.a   + (1.0f - accumulated_color.a) * texSample.a;
		}

		if (accumulated_color.a > 0.95) // early exit
			break;
	}

	accumulated_color.rgb = pow(accumulated_color.rgb, vec3(1.0f / 2.2f));
	o_color = accumulated_color;
}
