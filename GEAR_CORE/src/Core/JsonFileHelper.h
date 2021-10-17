#pragma once
#include "gear_core_common.h"

namespace gear
{
namespace core
{
	void LoadJsonFile(std::string& filepath, const std::string& fileExt, const std::string& fileTypeString, nlohmann::json& jsonData);
	void SaveJsonFile(std::string& filepath, const std::string& fileExt, const std::string& fileTypeString, nlohmann::json& jsonData);
}
}