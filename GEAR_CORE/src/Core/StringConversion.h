#pragma once

#include "gear_core_common.h"

namespace gear
{
namespace core
{
	std::string to_string(const std::wstring& wstring);
	std::wstring to_wstring(const std::string& string);
}
}