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