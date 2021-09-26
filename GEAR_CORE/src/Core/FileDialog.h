#pragma once
#include "gear_core_common.h"

namespace gear
{
namespace core
{
	std::string FileDialog_Open(void* window, const char* filter);
	std::string FileDialog_Save(void* window, const char* filter);
}
}