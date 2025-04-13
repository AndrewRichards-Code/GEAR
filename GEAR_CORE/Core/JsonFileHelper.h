#pragma once
#include "gear_core_common.h"
#include "ARC/External/json/json.hpp"

namespace gear
{
	namespace core
	{
		GEAR_CORE_API void LoadJsonFile(const std::filesystem::path& filepath, const std::filesystem::path& fileExt, const std::string& fileTypeString, nlohmann::json& jsonData);
		GEAR_CORE_API void SaveJsonFile(const std::filesystem::path& filepath, const std::filesystem::path& fileExt, const std::string& fileTypeString, nlohmann::json& jsonData);
	}
}