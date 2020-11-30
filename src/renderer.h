namespace renderer
{
	void init(GLFWwindow* window);
	void uninit();

	void render(GLFWwindow* window, Scene& scene, float dt);

	Camera& get_camera(); // @Cleanup should be in app or scene

	void render_ui();
}