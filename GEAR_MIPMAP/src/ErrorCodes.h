#pragma once

namespace gear
{
namespace mipmap
{
	//Return values from the main function
	enum class ErrorCode : int
	{
		GEAR_MIPMAP_OK = 0,
		GEAR_MIPMAP_ERROR,
		GEAR_MIPMAP_NO_ARGS,
		GEAR_MIPMAP_NO_OUTPUT_FILE_FORMAT,
		GEAR_MIPMAP_NO_IMAGE_FILE,
		GEAR_MIPMAP_IMAGE_FILE_INVALID,
		GEAR_MIPMAP_IMAGE_FILE_NOT_POW_OF_2,
		GEAR_MIPMAP_IMAGE_FILE_SAVE_ERROR,
		GEAR_MIPMAP_MIRU_ERROR,
		GEAR_MIPMAP_MIRU_NO_GRAPHICS_API,
	};

	inline std::string ErrorCodeStr(ErrorCode code)
	{
		switch (code)
		{
		default:
		case ErrorCode::GEAR_MIPMAP_OK:
			return "GEAR_MIPMAP_OK";
		case ErrorCode::GEAR_MIPMAP_ERROR:
			return "GEAR_MIPMAP_ERROR";
		case ErrorCode::GEAR_MIPMAP_NO_ARGS:
			return "GEAR_MIPMAP_NO_ARGS";
		case ErrorCode::GEAR_MIPMAP_NO_OUTPUT_FILE_FORMAT:
			return "GEAR_MIPMAP_NO_OUTPUT_FILE_FORMAT";
		case ErrorCode::GEAR_MIPMAP_NO_IMAGE_FILE:
			return "GEAR_MIPMAP_NO_IMAGE_FILE";
		case ErrorCode::GEAR_MIPMAP_IMAGE_FILE_NOT_POW_OF_2:
			return "GEAR_MIPMAP_IMAGE_FILE_NOT_POW_OF_2";
		case ErrorCode::GEAR_MIPMAP_IMAGE_FILE_SAVE_ERROR:
			return "GEAR_MIPMAP_IMAGE_FILE_SAVE_ERROR";
		case ErrorCode::GEAR_MIPMAP_MIRU_ERROR:
			return "GEAR_MIPMAP_MIRU_ERROR";
		case ErrorCode::GEAR_MIPMAP_MIRU_NO_GRAPHICS_API:
			return "GEAR_MIPMAP_MIRU_NO_GRAPHICS_API";
		}
	}

	//Debugbreak and assert
	#ifdef _DEBUG
	#if defined(_MSC_VER)
	#define DEBUG_BREAK __debugbreak()
	#else
	#define DEBUG_BREAK raise(SIGTRAP)
	#endif
	#else
	#define DEBUG_BREAK
	#endif

	static bool output = true;

	//GEAR printf
	#define GEAR_MIPMAP_PRINTF(s, ...) if(output) {printf((s), __VA_ARGS__);}

	//Log error code
	#define GEAR_MIPMAP_RETURN(x, y) {if(x != gear::mipmap::ErrorCode::GEAR_MIPMAP_OK) { printf("GEAR_MIPMAP_ASSERT: %s(%d): [%s] %s\n", __FILE__, __LINE__, ErrorCodeStr(x).c_str(), y); DEBUG_BREAK; } return static_cast<int>(x); } 
	#define GEAR_MIPMAP_ERROR_CODE(x, y) {if(x != gear::mipmap::ErrorCode::GEAR_MIPMAP_OK) { printf("GEAR_MIPMAP_ASSERT: %s(%d): [%s] %s\n", __FILE__, __LINE__, ErrorCodeStr(x).c_str(), y); DEBUG_BREAK; } } 

	
}
}