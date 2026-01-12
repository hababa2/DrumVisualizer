#pragma once

typedef unsigned char U8;		//Unsigned 8-bit integer
typedef unsigned short U16;		//Unsigned 16-bit integer
typedef unsigned int U32;		//Unsigned 32-bit integer
typedef unsigned long UL32;		//Unsigned 32-bit integer
typedef unsigned long long U64;	//Unsigned 64-bit integer

typedef signed char I8;			//Signed 8-bit integer
typedef signed short I16;		//Signed 16-bit integer
typedef signed int I32;			//Signed 32-bit integer
typedef signed long L32;		//Signed 32-bit integer
typedef signed long long I64;	//Signed 64-bit integer

typedef float F32;				//32-bit floating point number
typedef double F64;				//64-bit floating point number

typedef char C8;				//8-bit ascii character
typedef char16_t C16;			//16-bit unicode character
typedef wchar_t CW;				//Platform defined wide character, WINDOWS: 16-bit, OTHER: 32-bit
typedef char32_t C32;			//32-bit unicode character

typedef decltype(__nullptr) NullPointer; //Nullptr type

static inline constexpr unsigned long long U64_MAX = U64(0xFFFFFFFFFFFFFFFF);	//Maximum value of an unsigned 64-bit integer
static inline constexpr unsigned long long U64_MIN = U64(0x0000000000000000);	//Minimum value of an unsigned 64-bit integer
static inline constexpr signed long long I64_MAX = I64(0x7FFFFFFFFFFFFFFF);		//Maximum value of a signed 64-bit integer
static inline constexpr signed long long I64_MIN = I64(0x8000000000000000);		//Minimum value of a signed 64-bit integer
static inline constexpr unsigned int U32_MAX = U32(0xFFFFFFFF);					//Maximum value of an unsigned 32-bit integer
static inline constexpr unsigned int U32_MIN = U32(0x00000000);					//Minimum value of an unsigned 32-bit integer
static inline constexpr signed int I32_MAX = I32(0x7FFFFFFF);					//Maximum value of a signed 32-bit integer
static inline constexpr signed int I32_MIN = I32(0x80000000);					//Minimum value of a signed 32-bit integer
static inline constexpr unsigned long UL32_MAX = UL32(0xFFFFFFFF);				//Maximum value of an unsigned 32-bit integer
static inline constexpr unsigned long UL32_MIN = UL32(0x00000000);				//Minimum value of an unsigned 32-bit integer
static inline constexpr signed long L32_MAX = L32(0x7FFFFFFF);					//Maximum value of a signed 32-bit integer
static inline constexpr signed long L32_MIN = L32(0x80000000);					//Minimum value of a signed 32-bit integer
static inline constexpr unsigned short U16_MAX = U16(0xFFFF);					//Maximum value of an unsigned 16-bit integer
static inline constexpr unsigned short U16_MIN = U16(0x0000);					//Minimum value of an unsigned 16-bit integer
static inline constexpr signed short I16_MAX = I16(0x7FFF);						//Maximum value of a signed 16-bit integer
static inline constexpr signed short I16_MIN = I16(0x8000);						//Minimum value of a signed 16-bit integer
static inline constexpr unsigned char U8_MAX = U8(0xFF);						//Maximum value of an unsigned 8-bit integer
static inline constexpr unsigned char U8_MIN = U8(0x00);						//Minimum value of an unsigned 8-bit integer
static inline constexpr signed char I8_MAX = I8(0x7F);							//Maximum value of a signed 8-bit integer
static inline constexpr signed char I8_MIN = I8(0x80);							//Minimum value of a signed 8-bit integer
static inline constexpr float F32_MAX = 3.402823466e+38F;						//Maximum value of a 32-bit float
static inline constexpr float F32_MIN = 1.175494351e-38F;						//Minimum value of a 32-bit float
static inline constexpr double F64_MAX = 1.7976931348623158e+308;				//Maximum value of a 64-bit float
static inline constexpr double F64_MIN = 2.2250738585072014e-308;				//Minimum value of a 64-bit float

#if defined (WIN32) || defined (_WIN32) || defined (__WIN32__) || defined (__NT__)
#	define DV_PLATFORM_WINDOWS //Defined when on a Windows operating system
#	ifndef _WIN64
#		error "64-bit operating system is required!"
#	endif
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
#	define DV_PLATFORM_IOS_SIM //Defined when on an IOS simulator
#	define DV_PLATFORM_IOS //Defined when on an IOS
#elif TARGET_OS_MACCATALYST
#	define DV_PLATFORM_MACCATALYST //Defined when on an IOS
#elif TARGET_OS_IPHONE
#	define DV_PLATFORM_IPHONE //Defined when on an IOS
#elif TARGET_OS_MAC
#	define DV_PLATFORM_MAC //Defined when on an mac operating system
#else
#   error "Unknown Apple platform"
#endif
#elif __ANDROID__
#	define DV_PLATFORM_ANDROID //Defined when on an Android
#elif __linux__
#	define DV_PLATFORM_LINUX //Defined when on a Linux operating system
#elif __unix__
#	define DV_PLATFORM_UNIX //Defined when on a Unix operating system
#elif defined (_POSIX_VERSION)
#	define DV_PLATFORM_POSIX //Defined when on a Posix operating system
#else
#   error "Unknown compiler"
#endif

#if defined (DV_PLATFORM_WINDOWS) || defined (__LITTLE_ENDIAN__) || (defined (__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#	define DV_LITTLE_ENDIAN  //Defined when on a little endian operating system
#elif defined (__BIG_ENDIAN__) || (defined (__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#	define DV_BIG_ENDIAN //Defined when on a big endian operating system
#else
#	warning "could not determine endianness! Falling back to little endian..."
#	define DV_LITTLE_ENDIAN //Defined when on a little endian operating system
#endif

#ifdef _DEBUG
#	define DV_DEBUG				// Defined if running in debug mode
#else
#	define DV_RELEASE			// Defined if running in release mode
#endif

#define STATIC_CLASS(c)				\
c() = delete;						\
c(const c&) = delete;				\
c(c&&) = delete;					\
~c() = delete;						\
c& operator=(const c&) = delete;	\
c& operator=(c&&) = delete;

/// <summary>
/// Gets the element count of a static array
/// </summary>
/// <returns>The count of elements</returns>
template<class Type, U64 Count> constexpr U64 CountOf(Type(&)[Count]) { return Count; }

/// <summary>
/// Gets the element count of a static array
/// </summary>
/// <returns>The count of elements</returns>
template<class Type, U32 Count> constexpr U32 CountOf32(Type(&)[Count]) { return Count; }

/// <summary>
/// Creates a hash for a string literal at compile-time
/// </summary>
/// <param name="str:">The string literal</param>
/// <param name="length:">The length of the string</param>
/// <returns>The hash</returns>
static constexpr U64 Hash(const C8* str, U64 length)
{
	U64 hash = 5381;
	U64 i = 0;

	for (i = 0; i < length; ++str, ++i)
	{
		hash = ((hash << 5) + hash) + (*str);
	}

	return hash;
}

/// <summary>
/// Creates a hash for a string literal at compile-time, case insensitive
/// </summary>
/// <param name="str:">The string literal</param>
/// <param name="length:">The length of the string</param>
/// <returns>The hash</returns>
static constexpr U64 HashCI(const C8* str, U64 length)
{
	U64 hash = 5381;
	U64 i = 0;

	for (i = 0; i < length; ++str, ++i)
	{
		C8 c = *str;
		if (c > 64 && c < 91) { c += 32; }

		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

constexpr inline U64 operator""_Hash(const C8 * str, U64 length) { return Hash(str, length); }

struct Vector2
{
	F32 x, y;

	Vector2& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
	Vector2 operator*(F32 f) const { return { x * f, y * f }; }
	Vector2 operator*(const Vector2& v) const { return { x * v.x, y * v.y }; }
	Vector2 operator/(const Vector2& v) const { return { x / v.x, y / v.y }; }
	Vector2 operator+(const Vector2& v) const { return { x + v.x, y + v.y }; }
	Vector2 operator-(const Vector2& v) const { return { x - v.x, y - v.y }; }
};

struct Vector3
{
	F32 x, y, z;

	Vector3 operator*(F32 f) const { return { x * f, y * f, z * f }; }
	Vector3& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
};

struct Vector4
{
	F32 x, y, z, w;
};