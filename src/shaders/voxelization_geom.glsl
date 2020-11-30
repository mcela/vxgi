#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 u_shadowmap_mvp;

in vec4 g_world_pos[];
in vec3 g_normal[];
in vec2 g_tex_coords[];
out vec3 f_normal;
out vec2 f_tex_coords;
out vec3 f_voxel_pos; // world coordinates scaled to clip space (-1...1)
out vec4 f_shadow_coords;

void main()
{
	//
	// we're projecting the triangle face orthogonally with the dominant axis of its normal vector.
	// the end goal is to maximize the amount of generated fragments. more details at OpenGL Insights, pages 303-318
	//

	mat3 swizzle_mat;

	const vec3 edge1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	const vec3 edge2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	const vec3 face_normal = abs(cross(edge1, edge2)); 

	if (face_normal.x >= face_normal.y && face_normal.x >= face_normal.z) { // see: Introduction to Geometric Computing, page 33 (Ghali, 2008)
		swizzle_mat = mat3(
			vec3(0.0, 0.0, 1.0),
			vec3(0.0, 1.0, 0.0),
			vec3(1.0, 0.0, 0.0));
	} else if (face_normal.y >= face_normal.z) {
		swizzle_mat = mat3(
			vec3(1.0, 0.0, 0.0),
			vec3(0.0, 0.0, 1.0),
			vec3(0.0, 1.0, 0.0));
	} else {
		swizzle_mat = mat3(
			vec3(1.0, 0.0, 0.0),
			vec3(0.0, 1.0, 0.0),
			vec3(0.0, 0.0, 1.0));
	}

	for (int i=0; i < 3; i++)
	{
		gl_Position = vec4(gl_in[i].gl_Position.xyz * swizzle_mat, 1.0f);

		f_voxel_pos = gl_in[i].gl_Position.xyz;	
		f_shadow_coords = u_shadowmap_mvp * g_world_pos[i];
		f_normal = g_normal[i];
		f_tex_coords = g_tex_coords[i];

		EmitVertex();
	}

	EndPrimitive();
}
