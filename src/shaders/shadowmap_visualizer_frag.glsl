#version 450 core

uniform sampler2D u_tex_shadowmap;

in vec2 f_tex_coords;
out vec4 o_color;

void main()
{
	float d = texture(u_tex_shadowmap, f_tex_coords).r;
	o_color = vec4(d, d, d, 1.0f);
}