#version 450 core

// input is a full screen unit quad
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_color;
layout (location = 3) in vec2 v_tex_coords;

out vec2 f_tex_coords;

void main()
{
	f_tex_coords = v_tex_coords;
	gl_Position = vec4(v_position, 1.0f);
}

