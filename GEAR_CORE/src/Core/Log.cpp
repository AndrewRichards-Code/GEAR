#include "gear_core_common.h"
#include "Log.h"

#include <sstream>
#include "date.h"

#if defined(_WIN64)
#include <Windows.h>
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#define CONSOLE_OUTPUT_BRIGHT_RED SetConsoleTextAttribute(hConsole, 12)
#define CONSOLE_OUTPUT_RED SetConsoleTextAttribute(hConsole, 4)
#define CONSOLE_OUTPUT_YELLOW SetConsoleTextAttribute(hConsole, 6)
#define CONSOLE_OUTPUT_GREEN SetConsoleTextAttribute(hConsole, 2)
#define CONSOLE_OUTPUT_GREY SetConsoleTextAttribute(hConsole, 8)
#define CONSOLE_OUTPUT_WHITE SetConsoleTextAttribute(hConsole, 7)
#else
#define CONSOLE_OUTPUT_BRIGHT_RED
#define CONSOLE_OUTPUT_RED
#define CONSOLE_OUTPUT_YELLOW
#define CONSOLE_OUTPUT_GREEN
#define CONSOLE_OUTPUT_GREY
#define CONSOLE_OUTPUT_WHITE
#endif

using namespace gear;
using namespace core;

static constexpr size_t BufferSize = 1024Ui64;
static Log::Level s_Level = Log::Level::ALL;

const std::string Log::GenerateMessage(Level level, const char* __file__, int __line__, const char* __funcsig__, ErrorCode error, const char* format, ...)
{
	if (level == Level::NONE || (level & s_Level) == Level::NONE)
		return "";

	std::string buffer(BufferSize, 0);
	va_list args;
	va_start(args, format);
	vsprintf_s(&buffer[0], BufferSize, format, args);
	va_end(args);

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	auto day = date::floor<date::days>(now);
	auto ymd = date::year_month_day(day);
	auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(now - day));
	std::stringstream dateTimeSS;
	dateTimeSS << std::setfill('0');
	dateTimeSS << std::setw(4) << std::to_string(int(ymd.year()));
	dateTimeSS << std::setw(1) << "/";
	dateTimeSS << std::setw(2) << std::to_string(unsigned(ymd.month()));
	dateTimeSS << std::setw(1) << "/";
	dateTimeSS << std::setw(2) << std::to_string(unsigned(ymd.day()));
	dateTimeSS << std::setw(1) << " ";
	dateTimeSS << std::setw(2) << std::to_string(time.hours().count());
	dateTimeSS << std::setw(1) << ":";
	dateTimeSS << std::setw(2) << std::to_string(time.minutes().count());
	dateTimeSS << std::setw(1) << ":";
	dateTimeSS << std::setw(2) << std::to_string(time.seconds().count());
	dateTimeSS << std::setw(1) << ".";
	dateTimeSS << std::setw(3) << std::to_string(time.subseconds().count());
	dateTimeSS << " UTC";

	std::stringstream resultSS; 
	resultSS << "[" << dateTimeSS.str() << "][GEAR_CORE: " << LogTypeToString(level) << ": " << __file__ << "(" << std::to_string(__line__) << "): " << __funcsig__ << "][ErrorCode: " << ErrorCodeToString(error) << "]: " << buffer;
	
	return resultSS.str();
}

void Log::SetLevel(Level level)
{
	s_Level = level;
}

const std::string Log::LogTypeToString(Level level) noexcept
{
	switch (level)
	{
	default:
	case Log::Level::NONE:
		return "NONE";
	case Log::Level::FATAL:
		return "FATAL";
	case Log::Level::ERROR:
		return "ERROR";
	case Log::Level::WARN:
		return "WARN";
	case Log::Level::INFO:
		return "INFO";
	case Log::Level::ALL:
		return "ALL";
	};
}

const std::string Log::ErrorCodeToString(ErrorCode error) noexcept
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

void Log::SetConsoleOutputColour(Level level)
{
	switch (level)
	{
	default:
	case Log::Level::NONE:
		CONSOLE_OUTPUT_WHITE; return;
	case Log::Level::FATAL:
		CONSOLE_OUTPUT_BRIGHT_RED; return;
	case Log::Level::ERROR:
		CONSOLE_OUTPUT_RED; return;
	case Log::Level::WARN:
		CONSOLE_OUTPUT_YELLOW; return;
	case Log::Level::INFO:
		CONSOLE_OUTPUT_GREY; return;
	case Log::Level::ALL:
		CONSOLE_OUTPUT_WHITE; return;
	};
}
