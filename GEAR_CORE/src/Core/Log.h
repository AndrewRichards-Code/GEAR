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
			AUDIO,
			CORE,
			GRAPHICS,
			INPUT,
			OBJECTS,
			SCENE,
			UTILS,
			REVERSED,

			NO_DEVICE =  0x00010000,
			NO_CONTEXT,
			INIT_FAILED,
			FUNC_FAILED,
			NOT_SUPPORTED,
			INVALID_VALUE,
			NO_FILE,
			LOAD_FAILED,
			INVALID_COMPONENT,
			INVALID_PATH,
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