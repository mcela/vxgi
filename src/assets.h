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