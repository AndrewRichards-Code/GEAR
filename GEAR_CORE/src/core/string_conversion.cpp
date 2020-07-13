#include "gear_core_common.h"
#include "string_conversion.h"

using namespace gear;
using namespace core;

std::string to_string(const std::wstring& wstring)
{
	return std::string(wstring.begin(), wstring.end());
}

std::wstring to_wstring(const std::string& string)
{
	return std::wstring(string.begin(), string.end());
}