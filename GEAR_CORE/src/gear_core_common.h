#pragma once
#if defined(_MSC_VER)
#pragma warning(disable : 26812) //Disables 'Prefered scoped enum'  warning C26812
#pragma warning(disable : 26495) //Disables 'Unitialised variable'  warning C26495
#pragma warning(disable : 26451) //Disables 'Arithmetic overflow'   warning C26451
#pragma warning(disable : 6011)  //Disables 'Dereferencing nullptr' warning C6011
#pragma warning(disable : 6001)  //Disables 'Uninitialized memory'  warning C6001
#pragma warning(disable : 4251)  //Disables 'DLL-interface needed'  warning C4251
#endif

//C Standard Libraries
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

//STL
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <algorithm>

//Smart Poiners
#include <memory>

//Removed Window Definition and Set Subsystem
#if defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifndef _DEBUG
#if _WIN64
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif
#endif

//Dependencies

//Assimp
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#ifdef _DEBUG
#pragma comment(lib, "ASSIMP/lib/Debug/assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "ASSIMP/lib/Release/assimp-vc142-mt.lib")
#endif

//FreeType
#include "ft2build.h"
#include FT_FREETYPE_H
#pragma comment(lib, "FREETYPE/win64/freetype.lib")

//GLFW
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#pragma comment(lib, "GLFW/lib-vc2019/glfw3.lib")

//JSON
#include "json.hpp"

//MARS
#include "mars.h"
#ifdef _DEBUG
#pragma comment(lib, "MARS/MARS/lib/x64/Debug/MARS.lib")
#else
#pragma comment(lib, "MARS/MARS/lib/x64/Release/MARS.lib")
#endif

//MIRU
#include "miru_core.h"
#ifdef _DEBUG
#pragma comment(lib, "MIRU/MIRU_CORE/lib/x64/Debug/MIRU_CORE.lib")
#else
#pragma comment(lib, "MIRU/MIRU_CORE/lib/x64/Release/MIRU_CORE.lib")
#endif

//OpenAL
#include "AL/al.h"
#include "AL/alc.h"
#pragma comment(lib, "OPENAL/libs/Win64/OpenAL32.lib")

//GEAR Helpers
namespace gear
{
	template<class T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<class T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<class _Ty1, class _Ty2>
	inline constexpr Ref<_Ty1> ref_cast(const Ref<_Ty2>& x) noexcept { return std::dynamic_pointer_cast<_Ty1>(x); }

	enum class GEAR_ERROR_CODE : uint32_t
	{
		GEAR_OK				= 0x00000000,
		GEAR_AUDIO			= 0x00000001,
		GEAR_CORE			= 0x00000002,
		GEAR_GRAPHICS		= 0x00000004,
		GEAR_INPUT			= 0x00000008,
		GEAR_OBJECTS		= 0x00000010,
		GEAR_UTILS			= 0x00000020,
		GEAR_REVERSED0		= 0x00000040,
		GEAR_REVERSED1		= 0x00000080,

		GEAR_NO_DEVICE		= 0x00000100,
		GEAR_NO_CONTEXT		= 0x00000200,
		GEAR_INIT_FAILED	= 0x00000400,
		GEAR_FUNC_FAILED	= 0x00000800,
		GEAR_NOT_SUPPORTED	= 0x00001000,
		GEAR_INVALID_VALUE	= 0x00002000,
		GEAR_NO_FILE		= 0x00004000,
		GEAR_LOAD_FAILED	= 0x00008000,
	};
}

//GEAR printf
#if defined(_DEBUG)
#if !defined(__ANDROID__)
#define GEAR_PRINTF(s, ...) printf((s), __VA_ARGS__)
#else
#define GEAR_PRINTF(s, ...) __android_log_print(ANDROID_LOG_DEBUG, "GEAR_CORE", s, __VA_ARGS__)
#endif
#else
#define GEAR_PRINTF(s, ...) printf("")
#endif

//Triggered if x != 0
#define GEAR_ASSERT(x, y) if(static_cast<int>(x) != 0) { GEAR_PRINTF("GEAR_ASSERT: %s(%d): ERROR_CODE: %d(0x%x) - %s\n", __FILE__, __LINE__, static_cast<int>(x), static_cast<int>(x), y); DEBUG_BREAK; }
#define GEAR_WARN(x, y) if(static_cast<int>(x) != 0) { GEAR_PRINTF("GEAR_WARN: %s(%d): ERROR_CODE: %d(0x%x) - %s\n", __FILE__, __LINE__, static_cast<int>(x), static_cast<int>(x), y); }