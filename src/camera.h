struct Camera
{
	vec3 up = { 0,1,0 };
	vec3 direction = { 0,0,-1 };
	vec3 position = { 0,0,0 };

	mat4 view = mat4(1.0f);
	mat4 projection = mat4(1.0f);
	mat4 VP = mat4(1.0f);
};

struct Fly_Camera_Controls
{
	float yaw = 0.0f;
	float pitch = 0.0f;
	bool is_first_update = true;

	Camera* camera = NULL;
	Camera lerp_state;
};

namespace camera
{
	void update(Camera& c);

	void set_to_perspective(Camera& c, float aspect_ratio, float fov = 45.0f, float near_plane = 0.1f, float far_plane = 1000.0f);
	void set_to_ortho(Camera& c);

	vec3 get_right(Camera& c);
	vec3 get_forward(Camera& c);
}

namespace flycamera
{
	void attach(Fly_Camera_Controls& c, Camera* camera);
	void update(Fly_Camera_Controls& c, GLFWwindow* window, float dt);
}