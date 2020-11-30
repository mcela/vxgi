#version 450 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_color;
layout (location = 3) in vec2 v_tex_coord;

out vec2 f_tex_coord;

uniform mat4 VP;

void main()
{
	f_tex_coord = v_tex_coord;
	f_tex_coord.y = 1.0 - f_tex_coord.y;

	gl_Position = VP * vec4(v_position, 1.0);
}
