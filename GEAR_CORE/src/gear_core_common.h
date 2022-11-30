#pragma once
#if defined(_MSC_VER)
#pragma warning(disable : 26812) //Disables 'Prefered scoped enum'        warning C26812
#pragma warning(disable : 26495) //Disables 'Unitialised variable'        warning C26495
#pragma warning(disable : 26451) //Disables 'Arithmetic overflow'         warning C26451
#pragma warning(disable : 4201)  //Disables 'Nameless struct/union'       warning C4201
#pragma warning(disable : 4505)  //Disables 'Unreferenced local function' warnign C4505
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
#include <set>
#include <map>
#include <algorithm>
#include <functional>
#include <future>

//Smart Poiners
#include <memory>

//Platfrom Macros
#include "Core/PlatformMacros.h"

//GEAR Version
#define GEAR_VERSION_MAJOR 1
#define GEAR_VERSION_MINOR 0
#define GEAR_VERSION_PATCH 0
#define GEAR_MAKE_VERSION(major, minor, patch) ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))
#define GEAR_VERSION_CURRENT GEAR_MAKE_VERSION(GEAR_VERSION_MAJOR, GEAR_VERSION_MINOR, GEAR_VERSION_PATCH)

#define GEAR_VERSION_TYPE_STR_ALPHA "Alpha"
#define GEAR_VERSION_TYPE_STR_BETA "Beta"
#define GEAR_VERSION_TYPE_STR_RELEASE "Release"
#define GEAR_VERSION_TYPE_STR GEAR_VERSION_TYPE_STR_ALPHA

#if defined(GEAR_PLATFORM_WINDOWS_X64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#endif

//Dependencies

//ARC
#include "ARC/src/ExportAttributes.h"

//Assimp
#include "ASSIMP/include/assimp/Importer.hpp"
#include "ASSIMP/include/assimp/scene.h"
#include "ASSIMP/include/assimp/postprocess.h"

//ENTT
#include "ENTT/entt.hpp"

//FreeType
#include "FREETYPE/include/ft2build.h"
#include FT_FREETYPE_H

//GLFW
#include "GLFW/include/GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/include/GLFW/glfw3native.h"

//ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/backends/imgui_impl_dx12.h"

//JSON
#include "JSON/json.hpp"

//MARS
#include "MARS/src/mars.h"

//MIRU
#include "MIRU/MIRU_CORE/src/miru_core.h"

//OpenAL
#include "OPENAL/include/AL/al.h"
#include "OPENAL/include/AL/alc.h"

//XAudio2
#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
#define XAUDIO2_HELPER_FUNCTIONS
#include <xaudio2.h>
#include <x3daudio.h>
#endif

//XInput
#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
#include <xinput.h>
#endif

//GEAR API
#ifdef GEAR_BUILD_DLL
#define GEAR_API ARC_EXPORT
#else
#define GEAR_API ARC_IMPORT
#endif

//GEAR Helpers
namespace gear
{
	enum class ErrorCode : uint32_t
	{
		OK = 0x00000000,
		ANIMATION,
		AUDIO,
		BUILD,
		CORE,
		GRAPHICS,
		INPUT,
		OBJECTS,
		SCENE,
		UI,
		UTILS,
	
		NO_DEVICE = 0x00010000,
		NO_CONTEXT,
		INIT_FAILED,
		FUNC_FAILED,
		NOT_SUPPORTED,
		INVALID_VALUE,
		NO_FILE,
		LOAD_FAILED,
		INVALID_COMPONENT,
		INVALID_PATH,
		INVALID_STATE,
	};

	static std::string ErrorCodeToString(int64_t error)
	{
		uint32_t group = uint32_t(error) & 0x0000FFFF;
		uint32_t errorType = uint32_t(error) & 0xFFFF0000;
		std::string str = "";
	
		switch (ErrorCode(group))
		{
		case ErrorCode::OK:
			str += "OK"; break;
		case ErrorCode::ANIMATION:
			str += "ANIMATION"; break;
		case ErrorCode::AUDIO:
			str += "AUDIO"; break;
		case ErrorCode::BUILD:
			str += "BUILD"; break;
		case ErrorCode::CORE:
			str += "CORE"; break;
		case ErrorCode::GRAPHICS:
			str += "GRAPHICS"; break;
		case ErrorCode::INPUT:
			str += "INPUT"; break;
		case ErrorCode::OBJECTS:
			str += "OBJECTS"; break;
		case ErrorCode::SCENE:
			str += "SCENE"; break;
		case ErrorCode::UI:
			str += "UI"; break;
		case ErrorCode::UTILS:
			str += "UTILS"; break;
		default:
			break;
		}
	
		str += " | ";
	
		switch (ErrorCode(errorType))
		{
		case ErrorCode::NO_DEVICE:
			str += "NO_DEVICE"; break;
		case ErrorCode::NO_CONTEXT:
			str += "NO_CONTEXT"; break;
		case ErrorCode::INIT_FAILED:
			str += "INIT_FAILED"; break;
		case ErrorCode::FUNC_FAILED:
			str += "FUNC_FAILED"; break;
		case ErrorCode::NOT_SUPPORTED:
			str += "NOT_SUPPORTED"; break;
		case ErrorCode::INVALID_VALUE:
			str += "INVALID_VALUE"; break;
		case ErrorCode::NO_FILE:
			str += "NO_FILE"; break;
		case ErrorCode::LOAD_FAILED:
			str += "LOAD_FAILED"; break;
		case ErrorCode::INVALID_COMPONENT:
			str += "INVALID_COMPONENT"; break;
		case ErrorCode::INVALID_PATH:
			str += "INVALID_PATH"; break;
		case ErrorCode::INVALID_STATE:
			str += "INVALID_STATE"; break;
		default:
			break;
		}
	
		return str;
	}
}

//GEAR DebugName
template<typename T>
std::string GenereateDebugName(const std::string& name)
{
	std::string _typenameFull = typeid(T).name();
	std::string _typename = _typenameFull.substr(_typenameFull.find_last_of("::") + 2);
	return "GEAR: " + _typename + ": " + name;
}

//GEAR logging

inline arc::Log GearLog("GEAR_CORE");
#ifdef ARC_LOG_INSTANCE
#undef ARC_LOG_INSTANCE
#define ARC_LOG_INSTANCE GearLog
#endif
#define GEAR_SET_ERROR_CODE_TO_STRING_FUNCTION GearLog.SetErrorCodeToStringFunction(ErrorCodeToString)

//Triggered if x != 0
#define GEAR_ASSERT(errorCode, fmt, ...) if(static_cast<int64_t>(errorCode) != 0) { ARC_FATAL(static_cast<int64_t>(errorCode), fmt, __VA_ARGS__);  ARC_ASSERT(false); }

#define GEAR_FATAL(errorCode, fmt, ...) if(static_cast<int64_t>(errorCode) != 0) { ARC_FATAL(static_cast<int64_t>(errorCode), fmt, __VA_ARGS__); }
#define GEAR_ERROR(errorCode, fmt, ...) if(static_cast<int64_t>(errorCode) != 0) { ARC_ERROR(static_cast<int64_t>(errorCode), fmt, __VA_ARGS__); }
#define GEAR_WARN(errorCode, fmt, ...) if(static_cast<int64_t>(errorCode) != 0) { ARC_WARN(static_cast<int64_t>(errorCode), fmt, __VA_ARGS__); }
#define GEAR_INFO(errorCode, fmt, ...) if(static_cast<int64_t>(errorCode) != 0) { ARC_INFO(static_cast<int64_t>(errorCode), fmt, __VA_ARGS__); }

