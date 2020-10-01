#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>

#ifdef ERROR //Found in wingdi.h
#undef ERROR
#endif

namespace gear 
{
namespace core 
{
	class Log
	{
	public:
		enum class Level : uint32_t
		{
			NONE = 0x00000000,
			FATAL = 0x00000001,
			ERROR = 0x00000002,
			WARN = 0x00000004,
			INFO = 0x00000008,
			ALL = 0x000000FF,
		};

		enum class ErrorCode : uint32_t
		{
			OK					= 0x00000000,
			AUDIO				= 0x00000001,
			CORE				= 0x00000002,
			GRAPHICS			= 0x00000003,
			INPUT				= 0x00000004,
			OBJECTS				= 0x00000005,
			SCENE				= 0x00000006,
			UTILS				= 0x00000007,
			REVERSED			= 0x00000008,

			NO_DEVICE			= 0x00010000,
			NO_CONTEXT			= 0x00020000,
			INIT_FAILED			= 0x00030000,
			FUNC_FAILED			= 0x00040000,
			NOT_SUPPORTED		= 0x00050000,
			INVALID_VALUE		= 0x00060000,
			NO_FILE				= 0x00070000,
			LOAD_FAILED			= 0x00080000,
			INVALID_COMPONENT	= 0x00090000,
			INVALID_PATH		= 0x000A0000,
			INVALID_STATE		= 0x000B0000,
		};

	public:
		template<typename... Args>
		static void PrintMessage(Level level, const char* __file__, int __line__, const char* __funcsig__, ErrorCode error, const char* format, Args&&... args)
		{
			SetConsoleOutputColour(level);
			std::string msg = GenerateMessage(level, __file__, __line__, __funcsig__, error, format, std::forward<Args>(args)...);
			if (!msg.empty())
			{
				GEAR_PRINTF("%s\n", msg.c_str());
			}
			SetConsoleOutputColour(Level::NONE);
		}
		static const std::string GenerateMessage(Level level, const char* __file__, int __line__, const char* __funcsig__, ErrorCode error, const char* format, ...);
		static void SetLevel(Level level = Level::NONE);

	private:
		static const std::string LogTypeToString(Level level) noexcept;
		static const std::string ErrorCodeToString(ErrorCode error) noexcept;
		static void SetConsoleOutputColour(Level level);
	};
}
}