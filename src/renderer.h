enum Render_Mode : int
{
	RENDER_VOXELIZED_SCENE,
	RENDER_SCENE,
	RENDER_SHADOW_MAP
};

namespace renderer
{
	void init(GLFWwindow* window);
	void uninit();

	void render(GLFWwindow* window, Scene& scene, float dt);

	Camera& get_camera(); // @Cleanup should be in app or scene

	void render_ui();
}