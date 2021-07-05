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
#include <future>

//Smart Poiners
#include <memory>

//Platfrom Macros
#include "Core/PlatformMacros.h"

//Removed Window Definition and Set Subsystem

#if defined(GEAR_PLATFORM_WINDOWS_X64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX

#ifndef _DEBUG
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
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

//XAudio2
#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
#define XAUDIO2_HELPER_FUNCTIONS
#include <xaudio2.h>
#include <x3daudio.h>
#pragma comment(lib,"xaudio2.lib") 
#endif

//XInput
#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
#include <xinput.h>
#pragma comment(lib,"xinput.lib") 
#endif

//GEAR Helpers
namespace gear
{
	enum class ErrorCode : uint32_t
	{
		OK = 0x00000000,
		AUDIO = 0x00000001,
		CORE = 0x00000002,
		GRAPHICS = 0x00000003,
		INPUT = 0x00000004,
		OBJECTS = 0x00000005,
		SCENE = 0x00000006,
		UTILS = 0x00000007,
		REVERSED = 0x00000008,
	
		NO_DEVICE = 0x00010000,
		NO_CONTEXT = 0x00020000,
		INIT_FAILED = 0x00030000,
		FUNC_FAILED = 0x00040000,
		NOT_SUPPORTED = 0x00050000,
		INVALID_VALUE = 0x00060000,
		NO_FILE = 0x00070000,
		LOAD_FAILED = 0x00080000,
		INVALID_COMPONENT = 0x00090000,
		INVALID_PATH = 0x000A0000,
		INVALID_STATE = 0x000B0000,
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
		case ErrorCode::AUDIO:
			str += "AUDIO"; break;
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
		case ErrorCode::UTILS:
			str += "UTILS"; break;
		case ErrorCode::REVERSED:
			str += "REVERSED1"; break;
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

//GEAR printf and logging
#define GEAR_PRINTF ARC_PRINTF

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

