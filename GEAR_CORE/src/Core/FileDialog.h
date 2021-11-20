#pragma once
#include "gear_core_common.h"

namespace gear
{
namespace core
{
	std::string FileDialog_Open(const char* filterName, const char* filterSpec);
	std::string FileDialog_Save(const char* filterName, const char* filterSpec);

	std::string FolderDialog_Browse();
}
}