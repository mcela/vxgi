#version 450 core

layout(location = 0) in vec3 v_position;
out vec2 f_tex_coords;

void main()
{
	f_tex_coords = (v_position.xy + vec2(1.0f)) * 0.5f; // -1...-1 to 0...1
	gl_Position = vec4(v_position, 1);
}
