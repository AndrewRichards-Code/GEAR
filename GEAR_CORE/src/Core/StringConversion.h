#pragma once

#include "gear_core_common.h"

namespace gear
{
namespace core
{
	std::string to_string(const std::wstring& wstring);
	std::wstring to_wstring(const std::string& string);

	std::string toupper(const std::string& string);
	std::wstring towupper(const std::wstring& wstring);

	std::string tolower(const std::string& string);
	std::wstring towlower(const std::wstring& wstring);
}
}