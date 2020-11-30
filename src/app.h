struct Resolution
{
	vec2 window;
	vec2 internal;

	float windowAspectRatio = 1.0f;
	float internalAspectRatio = 1.0f;

	vec2 position0;
	vec2 position1;
};

namespace resolution
{
	Resolution& get();
	void        set(vec2 windowResolution, vec2 internalRenderResolution);
	void        scale_with_black_bars();
	void        window_size_changed(int width, int height);
}

namespace application
{
	bool init(int argc, const char* argv[], const vec2& internal_render_resolution = vec2(1920, 1080));
	void deinit();
	void run();

	std::string read_file(const char* filepath);
}