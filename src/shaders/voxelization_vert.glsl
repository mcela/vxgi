#version 450 core

uniform mat4 M;
uniform mat4 N; // (normal matrix)
uniform vec3 u_scene_voxel_scale;

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_color; 
layout (location = 3) in vec2 v_tex_coords; 

out vec4 g_world_pos;
out vec3 g_normal;
out vec3 g_color;
out vec2 g_tex_coords;

void main()
{
	g_world_pos = M * vec4(v_position, 1.0f);
	g_normal = normalize(vec3(N * vec4(v_normal, 0.0)));
	g_color = v_color;
	g_tex_coords = v_tex_coords;

	gl_Position = vec4(g_world_pos.xyz * u_scene_voxel_scale, 1.0f); // we want to voxelize everything so the whole world is scaled to be inside clip space (-1.0...1.0)
}