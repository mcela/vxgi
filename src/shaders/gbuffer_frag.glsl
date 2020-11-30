#version 450 core

struct Material
{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Ns;
	vec3 Ke;
	float d;
	float Ni;
	vec3 Tr;
};

uniform Material u_material;
uniform sampler2D u_tex_ambient;
uniform sampler2D u_tex_diffuse;
uniform sampler2D u_tex_specular;
uniform sampler2D u_tex_emission;
uniform sampler2D u_tex_bumpmap;

in vec3 f_world_pos;
in vec3 f_normal;
in vec2 f_tex_coords;
in vec3 f_tangent;
in vec3 f_bitangent;
in mat3 fTBN;

// gbuffer textures
layout (location = 0) out vec4 o_tex_position;
layout (location = 1) out vec4 o_tex_normal;
layout (location = 2) out vec4 o_tex_bump;
layout (location = 3) out vec4 o_tex_albedo;
layout (location = 4) out vec4 o_tex_specular; // alpha component = shininess
//layout (location = 4) out vec4 o_tex_emission;

void main()
{
	vec4 ambient = vec4(u_material.Ka, 1.0) * texture(u_tex_ambient, f_tex_coords);
	vec4 diffuse = vec4(u_material.Kd, 1.0) * texture(u_tex_diffuse, f_tex_coords);
	vec4 specular = vec4(u_material.Ks, 1.0) * texture(u_tex_specular, f_tex_coords);
	vec4 emission = vec4(u_material.Ke, 1.0) * texture(u_tex_emission, f_tex_coords);
	vec4 bump = texture(u_tex_bumpmap, f_tex_coords);

	vec3 normal = normalize(f_normal);
	vec3 bump_normal = ((bump.xyz - 0.5f) * 2.0f);
	bump_normal = (bump_normal.x * f_tangent) + (bump_normal.y * f_bitangent) + (bump_normal.z * normal);
	bump_normal = normalize(bump_normal);

	o_tex_position = vec4(f_world_pos, 1.0f);
	o_tex_normal = vec4(normal, 1.0f);
	o_tex_bump = vec4(bump_normal, 1.0f);
	o_tex_albedo = diffuse + emission; 
	o_tex_specular = vec4(specular.rgb, max(u_material.Ns, 0.06f));
//	o_tex_emission = emission;
}
