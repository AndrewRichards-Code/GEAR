#pragma once
#if defined(_MSC_VER)
#pragma warning(disable : 26812) //Disables 'Prefered scoped enum'  warning C26812
#pragma warning(disable : 26495) //Disables 'Unitialised variable'  warning C26495
#pragma warning(disable : 26451) //Disables 'Arithmetic overflow'   warning C26451
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

	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<class T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<class _Ty1, class _Ty2>
	inline constexpr Ref<_Ty1> ref_cast(const Ref<_Ty2>& x) noexcept { return std::dynamic_pointer_cast<_Ty1>(x); }
}

//GEAR printf
#if defined(_DEBUG)
#if !defined(__ANDROID__)
#define GEAR_PRINTF(fmt, ...) printf_s((fmt), __VA_ARGS__)
#else
#define GEAR_PRINTF(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, "GEAR_CORE", fmt, __VA_ARGS__)
#endif
#else
#define GEAR_PRINTF(fmt, ...) printf_s("")
#endif

#if(_WIN64)
#define GEAR_FUNCSIG __FUNCSIG__
#elif(__linux__)
#define GEAR_FUNCSIG __PRETTY_FUNCTION__
#endif

#include "Core/Log.h"
#if defined(_DEBUG)

#define GEAR_SET_LOG_LEVEL(level) gear::core::Log::SetLevel(gear::core::Log::Level(level))
#define GEAR_PRINT_MESSAGE(level, error_code, fmt, ...) gear::core::Log::PrintMessage(gear::core::Log::Level(level), __FILE__, __LINE__, GEAR_FUNCSIG, gear::core::Log::ErrorCode(error_code), fmt, __VA_ARGS__)

//Triggered if x != 0
#define GEAR_ASSERT(level, error_code, fmt, ...) if(static_cast<int>(error_code) != 0) { GEAR_PRINT_MESSAGE(level, error_code, fmt, __VA_ARGS__); DEBUG_BREAK; }
//Triggered if x != 0
#define GEAR_LOG(level, error_code, fmt, ...) if(static_cast<int>(error_code) != 0) { GEAR_PRINT_MESSAGE(level, error_code, fmt, __VA_ARGS__); }

#else

#define GEAR_SET_LOG_LEVEL(level)
#define GEAR_PRINT_MESSAGE(level, error_code, fmt, ...)

//Triggered if x != 0
#define GEAR_ASSERT(level, error_code, fmt, ...)
//Triggered if x != 0
#define GEAR_LOG(level, error_code, fmt, ...)

#endif
