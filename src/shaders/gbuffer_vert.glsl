#version 450 core

uniform mat4 M;
uniform mat4 N;
uniform mat4 VP;

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_color;
layout (location = 3) in vec2 v_tex_coords;
layout (location = 4) in vec3 v_tangent;
layout (location = 5) in vec3 v_bitangent;

out vec3 f_world_pos;
out vec3 f_normal;
out vec2 f_tex_coords;
out vec3 f_tangent;
out vec3 f_bitangent;
out mat3 fTBN;

void main()
{
	f_world_pos = (M * vec4(v_position, 1.0f)).xyz;
	f_normal = (N * vec4(v_normal, 1.0f)).xyz;
	f_tex_coords = v_tex_coords;
	f_tex_coords.y = 1.0 - f_tex_coords.y;

	vec3 T = normalize(vec3(M * vec4(v_tangent,   0.0)));
	vec3 B = normalize(vec3(M * vec4(v_bitangent, 0.0)));
	vec3 N = normalize(vec3(M * vec4(v_normal,    0.0)));
	fTBN = mat3(T, B, N);

	gl_Position = VP * vec4(f_world_pos, 1.0f);
}
