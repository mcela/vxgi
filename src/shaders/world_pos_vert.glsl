#version 450 core

layout(location = 0) in vec3 v_position;

uniform mat4 M;
uniform mat4 VP;

out vec3 f_world_pos;

void main()
{
	f_world_pos = vec3(M * vec4(v_position, 1.0f));
	gl_Position = VP * vec4(f_world_pos, 1.0f);
}