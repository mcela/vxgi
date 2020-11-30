#version 450 core

layout(location = 0) out float o_depth;

void main()
{
	o_depth = gl_FragCoord.z;
}