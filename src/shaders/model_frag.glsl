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
	vec3 Tf;
};

uniform Material u_material;
uniform sampler2D u_tex_ambient;
uniform sampler2D u_tex_diffuse;
uniform sampler2D u_tex_specular;
uniform sampler2D u_tex_emission;
uniform sampler2D u_tex_bumpmap;

in vec2 f_tex_coord;
out vec4 o_color;

void main()
{
	// @Note(13.05.2020): left here if needed in the future
	// vec4 ambient = vec4(u_material.Ka, 1.0) * texture(u_tex_ambient, f_tex_coord);
	// vec4 specular = vec4(u_material.Ks, 1.0) * texture(u_tex_specular, f_tex_coord);
	// vec4 emission = vec4(u_material.Ke, 1.0) * texture(u_tex_emission, f_tex_coord);
	// vec4 bump = texture(u_tex_bumpmap, f_tex_coord);

	vec4 diffuse = vec4(u_material.Kd, 1.0) * texture(u_tex_diffuse, f_tex_coord);
	o_color = diffuse;
}