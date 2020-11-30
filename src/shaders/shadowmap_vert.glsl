#version 450 core

layout (location = 0) in vec3 v_position;

uniform mat4 M;
uniform mat4 VP_shadow;

void main()
{
	gl_Position = VP_shadow * M * vec4(v_position, 1.0f);
}