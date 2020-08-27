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
			OK = 0x00000000,
			AUDIO = 0x00000001,
			CORE = 0x00000002,
			GRAPHICS = 0x00000004,
			INPUT = 0x00000008,
			OBJECTS = 0x00000010,
			UTILS = 0x00000020,
			REVERSED0 = 0x00000040,
			REVERSED1 = 0x00000080,

			NO_DEVICE = 0x00000100,
			NO_CONTEXT = 0x00000200,
			INIT_FAILED = 0x00000400,
			FUNC_FAILED = 0x00000800,
			NOT_SUPPORTED = 0x00001000,
			INVALID_VALUE = 0x00002000,
			NO_FILE = 0x00004000,
			LOAD_FAILED = 0x00008000,
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