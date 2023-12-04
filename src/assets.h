#pragma once

#include "containers.hpp"
#include "geometry.h"
#include "opengl.h"

namespace vxgi
{
	struct Asset_Manager
	{
		Array<Model*>     models;
		Array<Mesh*>      meshes;
		Array<Material*>  materials;
		Array<Texture2D*> textures;

		Texture2D         blank;
		Mesh              unit_cube;
		Mesh              unit_quad;
	};

	namespace assets
	{
		void init();
		void uninit();

		bool load(Array<Model*>& output_models, Bounding_Box& output_aabb, const char* path_to_obj, const char* path_to_textures);

		Material& get_material(int index);
		Texture2D& get_white_texture();

		Mesh& get_unit_cube();
		Mesh& get_unit_quad();
		void  generate_unit_cube(Mesh& out);
		void  generate_unit_quad(Mesh& out);
	}
}