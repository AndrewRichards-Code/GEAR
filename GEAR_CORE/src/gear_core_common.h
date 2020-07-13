#pragma once

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

//Dependencies
//MIRU
#include "miru_core.h"

//MARS
#include "mars.h"

//GLFW
#include "GLFW/x64/include/GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/x64/include/GLFW/glfw3native.h"

//OpenAL
#include "OPENAL/include/AL/al.h"
#include "OPENAL/include/AL/alc.h"

//FreeType
#include "FREETYPE/include/ft2build.h"
#include FT_FREETYPE_H

//Assimp
#include "ASSIMP/include/assimp/Importer.hpp"
#include "ASSIMP/include/assimp/scene.h"
#include "ASSIMP/include/assimp/postprocess.h"

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