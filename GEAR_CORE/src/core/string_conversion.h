#pragma once

#include <string>

namespace gear
{
namespace core
{
	//Parameter 'wstring' must only contain UTF-8/ASCII characters.
	std::string to_string(const std::wstring& wstring);

	//Parameter 'string' must only contain UTF-8/ASCII characters.
	std::wstring to_wstring(const std::string& string);
}
}