#pragma once

#include "types.h"

namespace vxgi
{
	struct Texture2D
	{
		std::string path;
		GLuint id = 0;
		int width = 0;
		int height = 0;
		bool is_loaded = false;
	};

	struct Texture3D
	{
		GLuint id = 0;
		int dimensions = 0;
		bool is_loaded = false;
	};

	struct Frame_Buffer
	{
		int width = 0;
		int height = 0;

		GLuint fbo_id = 0;
		GLuint color_texture_id = 0;
		GLuint depth_rbo_id = 0;

		bool is_inited = false;
		bool has_depth = false;
	};

	enum SHADER_TYPE
	{
		SHADER_TYPE_NULL,
		SHADER_TYPE_VERTEX   = GL_VERTEX_SHADER,
		SHADER_TYPE_FRAGMENT = GL_FRAGMENT_SHADER,
		SHADER_TYPE_GEOMETRY = GL_GEOMETRY_SHADER
	};
	struct Shader_Source
	{

		const char*   name = "";
		const GLchar* src = 0;
		SHADER_TYPE   type = SHADER_TYPE_NULL;
		GLuint        id = 0;
		bool          isCompiled = false;
	};
	struct Shader_Program
	{
		const char* name = "";
		GLuint      id = 0;
	};

	struct G_Buffer
	{
		enum Texture_Type : int
		{
			POSITION = 0,
			NORMAL,
			BUMP, // we use regular normals for cone tracing
			ALBEDO,
			SPECULAR,
			//EMISSION, // replaced with bump
			//DEPTH, // handled separately
			TOTAL_GBUFFER_TEXTURES
		};

		Texture2D textures[TOTAL_GBUFFER_TEXTURES];
		Texture2D depthTexture;

		GLuint fbo = 0;
		GLuint depthRenderbuffer = 0;
		GLuint previous_fbo = 0;

		int width = 0;
		int height = 0;
		bool isDepthRenderBufferCreated = false;
	};

	struct Shadow_Map
	{
		struct Config {
			int resolution;
			float ortho;
			float near_plane;
			float far_plane;
		};

		Config config;

		mat4 view = mat4(1.0);
		mat4 projection = mat4(1.0);
		mat4 VP = mat4(1.0);
		mat4 VP_biased = mat4(1.0);

		bool   is_fbo_created = false;
		GLuint depth_fbo_id = 0;
		GLuint depth_texture_id = 0;
	};

	namespace texture
	{
		void init(Texture2D&, const void* data, int w, int h, GLint internalFormat, GLenum format, GLenum type, GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT, bool generateMipmaps, bool attachToFrameBuffer, GLenum fboAttachment = GL_COLOR_ATTACHMENT0, GLuint fboAttachmentLevel = 0);
		void uninit(Texture2D&);
		void activate(Texture2D&, GLuint shader_id, const char* sampler_name, GLuint offset);
	}
	namespace texture3D
	{
		void init(Texture3D&, GLfloat* data, int dimensions);
		void uninit(Texture3D&);
		void activate(Texture3D&, GLuint shader_id, const char* samplerName, int textureLocation = 0);
		void deactivate();
		void clear(Texture3D&, const vec4& clearColor);
		void generate_mipmaps(Texture3D&);
		void fill_pixel(GLfloat* data, int x,int y,int z, int w,int h,int d);
		void fill_corners(GLfloat* data, int w, int h, int d);
	}
	namespace framebuffer
	{
		void init(Frame_Buffer&, int w, int h);
		void uninit(Frame_Buffer&);
		void init_with_depth(Frame_Buffer&, int w, int h);
		void activate_color_as_texture(Frame_Buffer&, const int shaderProgram, const char* glSamplerName, const int textureLocation = 0);
	}
	namespace shader
	{
		bool   init(Shader_Program&, const char* name, const char* path_to_vert, const char* path_to_frag, const char* path_to_geom = "");
		void   uninit(Shader_Program&);
		GLuint activate(Shader_Program&);
		void   deactivate();

		void source_init(Shader_Source&, const char* name, SHADER_TYPE type, const char* src);
		void source_uninit(Shader_Source&);
		bool source_compile(Shader_Source&);
		bool was_compilation_successful(Shader_Source&);
		void log_compile_error(Shader_Source&);
	}
	namespace gbuffer
	{
		void init(G_Buffer&, int framebufferWidth, int framebufferHeight, bool createDepthRenderBuffer = false);
		void uninit(G_Buffer&);
		void activate(G_Buffer&);
		void deactivate(G_Buffer&);
		void bind_as_textures(G_Buffer&, GLuint fbo, GLuint shader_id, int textureLocationOffset = 0);
		void blit_to_screen(G_Buffer&, float viewportWidth, float viewportHeight);
	}
	namespace shadowmap
	{
		void init(Shadow_Map&, const Shadow_Map::Config& config);
		void uninit(Shadow_Map&);

		void update(Shadow_Map&, const vec3& light_direction);

		void fbo_activate(Shadow_Map&);
		void fbo_deactivate(Shadow_Map&);
		void texture_activate(Shadow_Map&, GLuint shader_id, const char* sampler_name, int texture_location_offset = 0);
		void texture_deactivate(Shadow_Map&);
	}
}