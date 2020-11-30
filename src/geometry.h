struct Bounding_Box
{
	vec3 min_point = vec3(0);
	vec3 max_point = vec3(0);
	vec3 center = vec3(0);
};

struct Transform
{
	vec3 position = { 0,0,0 };
	vec3 scale = { 1,1,1 };
	vec3 rotation = { 0,0,0 };
	mat4 mtx = mat4(1.0f);
	mat4 normal_mtx = mat4(1.0f);
};

struct Vertex
{
	vec3 position = vec3(0);
	vec3 normal = vec3(0);
	vec3 color = vec3(0);
	vec2 tex_coord = vec2(0);
	vec3 tangent = vec3(0);
	vec3 bitangent = vec3(0);
};

struct Material
{
	int index = 0;

	union { vec3 ambient_reflectance; vec3 Ka; };
	union { Texture2D* ambient_map; Texture2D* map_Ka; }; // in sponza this is equivalent to diffuse map 

	union { vec3 diffuse_reflectance; vec3 Kd; };
	union { Texture2D* diffuse_map; Texture2D* map_Kd; };

	union { vec3 specular_reflectance; vec3 Ks; };
	union { Texture2D* specular_map; Texture2D* map_Ks; };
	union { float specular_exponent; float Ns; float shininess; }; // defines the focus of the specular highlight, 0...1000

	union { vec3 emission; vec3 Ke; };
	union { Texture2D* emission_map; Texture2D* map_Ke; };

	union { Texture2D* bump_map; Texture2D* map_bump; };

	union { float dissolve_factor; float d; /* float Tr; */ }; // 0.0...1.0 (Tr = 1.0-d )
	union { float optical_density; float IOR; float Ni; }; // 0.001...10.0
	union { vec3 transmission_filter; vec3 Tf; }; // vec3(0.0)...vec3(1.0)
};

struct Sub_Mesh
{
	// sub mesh is a subarray of mesh.vertices with a unique material
	// mesh, that contains all vertices and indices, is broken into 1...n sub meshes for each material
	int index; // to vertices
	int length; 
	int material_index; // to Assets_Old::materials
};

struct Mesh
{
	const char* name = "";

	bool is_loaded = false;
	bool is_static = false;

	GLuint vao = 0;
	GLuint vbo = 0;

	int vao_size = 0;

//	Array<Vertex> vertices; // not saved to ram, uploaded directly to gpu
	Array<Sub_Mesh> sub_meshes;
};

struct Model
{
	const char* name = "";
	Transform transform;
	Bounding_Box bounding_box;
	Array<Mesh*> meshes;
};

namespace boundingbox
{
	void update(Bounding_Box& aabb) {
		aabb.center = (aabb.max_point + aabb.min_point) * 0.5f;
	}
}
namespace transform
{
	void update(Transform& t) {
		t.mtx = glm::translate(t.position) * glm::mat4_cast(glm::quat(t.rotation)) * glm::scale(t.scale);
		t.normal_mtx = glm::transpose(glm::inverse(t.mtx));
	}
}
