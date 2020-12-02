#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader-master/tiny_obj_loader.h>

#include <lodepng-master/lodepng.h>
#include <lodepng-master/lodepng.cpp>

namespace
{
	Asset_Manager& get_asset_manager() {
		static Asset_Manager mgr;
		return mgr;
	}

	void       upload_mesh_to_gpu(Mesh& mesh, Array<Vertex>& vertex_buffer);
	bool       load_texture(Texture2D& out, const char* png_file, bool generate_mipmaps = false); // uploads to gpu aswell
	Texture2D* get_or_load_material_texture(const char* name, Hashmap<const char*, Texture2D*>& loaded_textures, const char* path_to_textures);
	void       set_material_from(Material& m, tinyobj::material_t& mat);
}

namespace assets
{
	void init()
	{
		load_texture(get_asset_manager().blank, "missing_texture.png");
		generate_unit_cube(get_asset_manager().unit_cube);
		generate_unit_quad(get_asset_manager().unit_quad);
	}
	void uninit()
	{
		// TODO
	}

	Material& get_material(int index) {
		ASSERT(index < array::size(get_asset_manager().materials), "assets", "index %d < %d", index, array::size(get_asset_manager().materials));
		return *get_asset_manager().materials[index];
	}

	bool load(Array<Model*>& output_models, Bounding_Box& output_aabb, const char* path, const char* path_to_textures)
	{
		LOG("assets", "loading obj file");
		Asset_Manager& assetmgr = get_asset_manager();

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> obj_materials;
		{
			std::string warn, error;
			bool ret = tinyobj::LoadObj(&attrib, &shapes, &obj_materials, &warn, &error, path, ""); // note: triangulates
			if (!warn.empty())  
				LOG("assets", "warning loading file (%s): %s", path, warn.c_str());
			if (!error.empty()) 
				LOG("assets", "error loading file (%s): %s", path, error.c_str());
			if (!ret) {
				LOG("assets", "couldn't load file (%s)", path);
				return false;
			}
		}

		LOG("assets", "loading textures");
		Hashmap<const char*, Material*> material_map;
		Hashmap<const char*, Texture2D*> textures;
		{
			array::ensure_capacity(assetmgr.materials, obj_materials.size());
			int material_index = array::size(get_asset_manager().materials);

			for (tinyobj::material_t& obj_material : obj_materials)
			{
				Material* material = new Material; // @Cleanup @Malloc
				array::add(get_asset_manager().materials, material);
				hashmap::insert(material_map, obj_material.name.c_str(), material);

				material->index = material_index;
				material_index++;

				set_material_from(*material, obj_material);
				material->map_Ka   = get_or_load_material_texture(obj_material.ambient_texname.c_str(), textures, path_to_textures);
				material->map_Kd   = get_or_load_material_texture(obj_material.diffuse_texname.c_str(), textures, path_to_textures);
				material->map_Ks   = get_or_load_material_texture(obj_material.specular_texname.c_str(), textures, path_to_textures);
				material->map_Ke   = get_or_load_material_texture(obj_material.emissive_texname.c_str(), textures, path_to_textures);
				material->map_bump = get_or_load_material_texture(obj_material.bump_texname.c_str(), textures, path_to_textures);
			}
		}

		LOG("assets", "processing meshes");
		vec3 scene_min_point = vec3(MAX_FLOAT_VALUE, MAX_FLOAT_VALUE, MAX_FLOAT_VALUE);
		vec3 scene_max_point = vec3(MIN_FLOAT_VALUE, MIN_FLOAT_VALUE, MIN_FLOAT_VALUE);
		{
			array::ensure_capacity(assetmgr.models, shapes.size());
			array::ensure_capacity(assetmgr.meshes, shapes.size());

			for (size_t s=0; s < shapes.size(); s++)
			{
				Model* model = new Model; // @Cleanup @Malloc
				array::add(assetmgr.models, model);

				Mesh* mesh = new Mesh; // @Cleanup @Malloc
				array::add(assetmgr.meshes, mesh);

				array::add(model->meshes, mesh);
				array::add(mesh->sub_meshes, {});
				int current_submesh_index = 0;

				// indices to vector::obj_materials 
				int current_material_index = 0; 
				int previous_material_index = 0;

				if (shapes[s].mesh.material_ids.size() > 0) {
					current_material_index = previous_material_index = shapes[s].mesh.material_ids[0];
				}

				Sub_Mesh& first_submesh = mesh->sub_meshes[0];
				first_submesh.index = 0;
				first_submesh.material_index = current_material_index;

				Array<Vertex> vertex_buffer;
				for (int i=0; i < shapes[s].mesh.indices.size(); i += 3)
				{
					previous_material_index = current_material_index;
					current_material_index = shapes[s].mesh.material_ids[i / 3];
					tinyobj::material_t& obj_material = obj_materials[current_material_index]; 

					if (current_material_index != previous_material_index) {
						LOG("assets", "material changed!");

						array::add(mesh->sub_meshes, {});
						current_submesh_index++;
						Sub_Mesh& new_submesh = mesh->sub_meshes[current_submesh_index];
						new_submesh.index = i;

						// note: indices to vector::obj_materials don't map directly to Asset_Manager::materials
						// because there could be other unrelated materials too, so we fetch the correct index
						// from the hash map we created earlier.
						Material* material = hashmap::get(material_map, obj_material.name.c_str());
						new_submesh.material_index = material->index;
					}

					for (int j = 0; j < 3; j++) // face vertices
					{
						tinyobj::index_t& idx = shapes[s].mesh.indices[i + j];

						tinyobj::real_t vx, vy, vz;
						tinyobj::real_t nx, ny, nz;
						tinyobj::real_t cr, cg, cb;
						tinyobj::real_t tx, ty;

						vx = attrib.vertices[3 * idx.vertex_index+0];
						vy = attrib.vertices[3 * idx.vertex_index+1];
						vz = attrib.vertices[3 * idx.vertex_index+2];

						scene_min_point.x = fmin(vx, scene_min_point.x);
						scene_min_point.y = fmin(vy, scene_min_point.y);
						scene_min_point.z = fmin(vz, scene_min_point.z);
						scene_max_point.x = fmax(vx, scene_max_point.x);
						scene_max_point.y = fmax(vy, scene_max_point.y);
						scene_max_point.z = fmax(vz, scene_max_point.z);

						if (attrib.normals.size() > 0) {
							nx = attrib.normals[3 * idx.normal_index+0];
							ny = attrib.normals[3 * idx.normal_index+1];
							nz = attrib.normals[3 * idx.normal_index+2];
						} else {
							LOG("assets", "NO NORMALS!");
							nx = 0.0;
							ny = 0.0;
							nz = 0.0;
						}

						if (attrib.colors.size() > 0) {
							cr = attrib.colors[3 * idx.vertex_index+0];
							cg = attrib.colors[3 * idx.vertex_index+1];
							cb = attrib.colors[3 * idx.vertex_index+2];
						} else {
							cr = obj_material.diffuse[0];
							cg = obj_material.diffuse[1];
							cb = obj_material.diffuse[2];
						}

						if (attrib.texcoords.size() > 0) {
							tx = attrib.texcoords[2 * idx.texcoord_index+0];
							ty = attrib.texcoords[2 * idx.texcoord_index+1];
						} else {
							bool doesHaveTexture = 
								!obj_material.ambient_texname.empty()  ||
								!obj_material.diffuse_texname.empty()  ||
								!obj_material.specular_texname.empty() ||
								!obj_material.emissive_texname.empty() ||
								!obj_material.bump_texname.empty();
							if (doesHaveTexture) {
								LOG("assets", "NO TEXCOORDS EVEN THOUGH HAS A TEXTURE!");
							}
							tx = 0.0;
							ty = 0.0;
						}

						array::add(vertex_buffer, (Vertex) {
							{ vx,vy,vz },
							{ nx, ny, nz },
							{ cr, cg, cb },
							{ tx, ty },
							{ 0.0f,0.0f,0.0f }, // tangent & bitangent calculated below 
							{ 0.0f,0.0f,0.0f }
						});
					}

					// calculate tangents & bitangents
					{
						Vertex& f0 = vertex_buffer[i + 0];
						Vertex& f1 = vertex_buffer[i + 1];
						Vertex& f2 = vertex_buffer[i + 2];

						vec3& v0 = f0.position;
						vec3& v1 = f1.position;
						vec3& v2 = f2.position;
						vec2& uv0 = f0.tex_coord;
						vec2& uv1 = f1.tex_coord;
						vec2& uv2 = f2.tex_coord;

						vec3 edge1 = v1 - v0;
						vec3 edge2 = v2 - v0;
						vec2 deltaUV1 = uv1 - uv0;
						vec2 deltaUV2 = uv2 - uv0;

						float f = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
						if (f == 0.0f) f = 1.0f;
						else f = 1.0f / f;

						vec3 tangent;
						tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
						tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
						tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
						normalize_vec3(tangent);

						vec3 bitangent;
						bitangent.x = f * (-deltaUV2.x * edge1.x - deltaUV1.x * edge2.x);
						bitangent.y = f * (-deltaUV2.x * edge1.y - deltaUV1.x * edge2.y);
						bitangent.z = f * (-deltaUV2.x * edge1.z - deltaUV1.x * edge2.z);
						normalize_vec3(bitangent);

						f0.tangent = tangent;
						f1.tangent = tangent;
						f2.tangent = tangent;
						f0.bitangent = bitangent;
						f1.bitangent = bitangent;
						f2.bitangent = bitangent;
					}

					Sub_Mesh& current_submesh = mesh->sub_meshes[current_submesh_index];
					current_submesh.length += 3;
				}

				upload_mesh_to_gpu(*mesh, vertex_buffer);
				array::clear(vertex_buffer);
			}
		}

		{
			output_aabb.min_point = scene_min_point;
			output_aabb.max_point = scene_max_point;
			boundingbox::update(output_aabb);

			for (Model* model : get_asset_manager().models) {
				array::add(output_models, model);
			}
		}

		hashmap::uninit(material_map);
		hashmap::uninit(textures);
		LOG("assets", "scene loaded");
		return true;
	}

	Texture2D& get_white_texture() {
		return get_asset_manager().blank;
	}
	Mesh& get_unit_cube() {
		return get_asset_manager().unit_cube;
	}
	Mesh& get_unit_quad() {
		return get_asset_manager().unit_quad;
	}

	void generate_unit_cube(Mesh& mesh)
	{
		static Vertex vertices[] = 
		{
			{ { -1, -1, 1 }, { 0.5, 1, 1 }, {0,0,0},{0,0},{0,0,0},{0,0,0} },
			{ { 1, -1, 1 }, { 1, 0.5, 1 }, {0,0,0},{0,0},{0,0,0},{0,0,0}},
			{ { 1, 1, 1 }, { 1, 0, 0 }, {0,0,0},{0,0},{0,0,0},{0,0,0}},
			{ { -1, 1, 1 }, { 0, 1, 1 }, {0,0,0},{0,0},{0,0,0},{0,0,0}},
			{ { -1, -1, -1 }, { 1, 1, 1 }, {0,0,0},{0,0},{0,0,0},{0,0,0}},
			{ { 1, -1, -1 }, { 1, 1, 1 }, {0,0,0},{0,0},{0,0,0},{0,0,0}},
			{ { 1, 1, -1 }, { 1, 1, 1 }, {0,0,0},{0,0},{0,0,0},{0,0,0}},
			{ { -1, 1, -1 }, { 1, 0, 1 }, {0,0,0},{0,0},{0,0,0},{0,0,0} }
		};

		static GLuint indices[] = 
		{
			0, 1, 2, 2, 3, 0, 3, 2, 6,
			6, 7, 3, 7, 6, 5, 5, 4, 7,
			4, 5, 1, 1, 0, 4, 4, 0, 3,
			3, 7, 4, 1, 5, 6, 6, 2, 1
		};

		Array<Vertex> vertexBuffer;
		defer { array::uninit(vertexBuffer); };

		int totalIndices = SIZE_OF_STATIC_ARRAY(indices);
		umm m = array::reserve_before_insert(vertexBuffer, totalIndices);
		for (int i=0; i < totalIndices; i++) {
			array::insert_reserved(vertexBuffer, vertices[indices[i]], m,i);
		}

		mesh.vao_size = array::size(vertexBuffer);
		array::add(mesh.sub_meshes, { 0, mesh.vao_size, 0 });
		mesh.is_loaded = true;

		upload_mesh_to_gpu(mesh, vertexBuffer);
	}
	void generate_unit_quad(Mesh& mesh)
	{
		Array<Vertex> vertexBuffer;
		defer { array::uninit(vertexBuffer); };

		array::add(vertexBuffer, (Vertex) { { -1, -1,  1 }, { 0, 0, 1 }, {0,0,0}, { 0, 0 }, {0,0,0}, {0,0,0} } );
		array::add(vertexBuffer, (Vertex) { {  1, -1,  1 }, { 0, 0, 1 }, {0,0,0}, { 1, 0 }, {0,0,0}, {0,0,0} } );
		array::add(vertexBuffer, (Vertex) { {  1,  1,  1 }, { 0, 0, 1 }, {0,0,0}, { 1, 1 }, {0,0,0}, {0,0,0} } );
		array::add(vertexBuffer, (Vertex) { { -1, -1,  1 }, { 0, 0, 1 }, {0,0,0}, { 0, 0 }, {0,0,0}, {0,0,0} } );
		array::add(vertexBuffer, (Vertex) { {  1,  1,  1 }, { 0, 0, 1 }, {0,0,0}, { 1, 1 }, {0,0,0}, {0,0,0} } );
		array::add(vertexBuffer, (Vertex) { { -1,  1,  1 }, { 0, 0, 1 }, {0,0,0}, { 0, 1 }, {0,0,0}, {0,0,0} } );

		mesh.vao_size = array::size(vertexBuffer);
		array::add(mesh.sub_meshes, { 0, mesh.vao_size, 0 });
		mesh.is_loaded = true;

		upload_mesh_to_gpu(mesh, vertexBuffer);
	}
}

namespace
{
	bool load_texture(Texture2D& out, const char* png_file, bool generate_mipmaps)
	{
		out.path = png_file;

		u32 error = 0;
		u32 w = 0;
		u32 h = 0;
		u8* data = NULL;

		error = lodepng_decode32_file(&data, &w, &h, png_file);
		defer { free(data); };

		ASSERT(error == 0, "assets", "error %u loading img '%s': %s", error, png_file, lodepng_error_text(error));

		if (!error && data) {
			texture::init(out, data, int(w),int(h), GL_RGBA,GL_RGBA,GL_UNSIGNED_BYTE, (generate_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR),GL_LINEAR, GL_REPEAT,GL_REPEAT, generate_mipmaps, false);
			LOG("assets", "loaded texture %s (id %u)", png_file, out.id);
			return true;
		} else {
			LOG("assets", "couldn't load texture %s", png_file);
			return false;
		}
	}

	Texture2D* get_or_load_material_texture(const char* name, Hashmap<const char*, Texture2D*>& loaded_textures, const char* path_to_textures)
	{
		if (name && strlen(name) > 0) {
			Texture2D* tex = NULL;

			if (hashmap::contains(loaded_textures, name)) {
				tex = hashmap::get(loaded_textures, name);
			} else {
				tex = new Texture2D; // @Cleanup @Malloc
				load_texture(*tex, name, false);
				hashmap::insert(loaded_textures, name, tex);
				array::add(get_asset_manager().textures, tex);
			}

			assert(tex);
			return tex;
		} else {
			return &assets::get_white_texture();
		}
	}

	void upload_mesh_to_gpu(Mesh& mesh, Array<Vertex>& vertex_buffer)
	{
		glGenVertexArrays(1, &mesh.vao);
		glGenBuffers(1, &mesh.vbo);
		glBindVertexArray(mesh.vao);

		int vertex_buffer_length = array::size(vertex_buffer);
		size_t vertex_size = sizeof(Vertex);

		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_buffer_length * vertex_size, vertex_buffer.data, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid*) offsetof(Vertex, position));
		glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid*) offsetof(Vertex, normal));
		glEnableVertexAttribArray(2); glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid*) offsetof(Vertex, color));
		glEnableVertexAttribArray(3); glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid*) offsetof(Vertex, tex_coord));
		glEnableVertexAttribArray(4); glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid*) offsetof(Vertex, tangent));
		glEnableVertexAttribArray(5); glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid*) offsetof(Vertex, bitangent));

		glBindVertexArray(0);
		check_gl_error();
	}

	void set_material_from(Material& m, tinyobj::material_t& mat)
	{
		using glm::make_vec3;
		m.Ka = make_vec3(mat.ambient);
		m.Kd = make_vec3(mat.diffuse);
		m.Ks = make_vec3(mat.specular);
		m.Ns = mat.shininess;
		m.Ke = make_vec3(mat.emission);
		m.d = mat.dissolve;
		m.Ni = mat.ior;
		m.Tf = make_vec3(mat.transmittance);
	}
}
