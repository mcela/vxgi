#pragma once

#include <cstddef> // size_t
#include <stdint.h> // types
#include <string> // std::string
#include <assert.h> // assert
#include <stdarg.h> // va_
#include <fstream> // ifstream, read file
#include <iostream> // log
#include <cmath> // sin, cos

#define GLEW_STATIC
#include <GL/glew.h>
namespace vxgi {
	void _check_gl_error(const char* file, int line);
}
#define check_gl_error() vxgi::_check_gl_error(__FILE__,__LINE__)

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

typedef unsigned char       u8;
typedef unsigned short int  u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef signed char         s8;
typedef signed short int    s16;
typedef signed int          s32;
typedef signed long long    s64;
typedef float               r32;
typedef double              r64;
typedef unsigned int        b32;
typedef size_t              umm;

static const float PI = 3.141592653589793238462643383f;
static const float PI2 = PI * 2;
static const float PI_HALF = PI * 0.5f;
static const float RADIANS_TO_DEGREES = 180.0 / PI;
static const float DEGREES_TO_RADIANS = PI / 180.0;
static const float FLOAT_ROUNDING_ERROR = 0.000001f;
static const float MIN_FLOAT_VALUE = -3.40282e+38;
static const float MAX_FLOAT_VALUE =  3.40282e+38;

inline void LOG(const char* tag, const char* message, ...)
{
	printf("[%s] ", tag);
	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end (args);
	printf("\n");
}

inline void normalize_vec3(vec3& v) // glm::normalize() was returning NaNs
{
	float len = glm::length(v);
	if (len != 0.0f) {
		v.x /= len;
		v.y /= len;
		v.z /= len;
	}
}

#define BUFFER_OFFSET(offset) ((void *)(offset))
#define ASSERT(expr,tag,...) \
	do { \
		if (expr) { \
		} else { \
			LOG(tag,__VA_ARGS__); \
			assert(expr); \
		} \
	} while(0);

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define CONCAT_INTERNAL(x,y) x##y
#define CONCAT(x,y) CONCAT_INTERNAL(x,y)

#define SIZE_OF_STATIC_ARRAY(array) (sizeof(array) / sizeof(array[0]))

// defer
// usage: `defer { do_something_at_the_end_of_scope(); };`
// author: Jonathan Blow, https://pastebin.com/SX3mSC9n (MIT licence)
template<typename T>
struct ExitScope {
	T lambda;
	ExitScope(T lambda) : lambda(lambda) {}
	~ExitScope() { lambda(); }
private:
	ExitScope& operator =(const ExitScope&);
};
class ExitScopeHelp {
public:
	template<typename T> ExitScope<T>
	operator+(T t) { return t; }
};
#define defer const auto& CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

namespace vxgi
{
	struct Application;
	struct Application_Config;
	struct Texture2D;
	struct Scene;
	struct Scene_Lights;
	struct Material;
	struct Mesh;
	struct Camera;
}

struct GLFWwindow;
