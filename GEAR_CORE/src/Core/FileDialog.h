#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace core
	{
		GEAR_API std::string FileDialog_Open(const char* filterName, const char* filterSpec);
		GEAR_API std::string FileDialog_Save(const char* filterName, const char* filterSpec);

		GEAR_API std::string FolderDialog_Browse();
	}
}