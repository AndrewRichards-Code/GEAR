#pragma once
#include "gear_core_common.h"

namespace gear
{
namespace core
{
	GEAR_API void LoadJsonFile(std::string& filepath, const std::string& fileExt, const std::string& fileTypeString, nlohmann::json& jsonData);
	GEAR_API void SaveJsonFile(std::string& filepath, const std::string& fileExt, const std::string& fileTypeString, nlohmann::json& jsonData);
}
}