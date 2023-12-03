#include "gear_core_common.h"
#include "Core/JsonFileHelper.h"

#include <fstream>
#include <filesystem>

using namespace nlohmann;

void gear::core::LoadJsonFile(const std::string& filepath, const std::string& fileExt, const std::string& fileTypeString, json& jsonData)
{
	std::string _filepath = filepath;
	if (_filepath.find(fileExt) == std::string::npos)
		_filepath += fileExt;

	std::ifstream file(_filepath, std::ios::binary);
	if (file.is_open())
	{
		file >> jsonData;
	}
	else
	{
		GEAR_ASSERT(ErrorCode::CORE | ErrorCode::NO_FILE, "Unable to open %s.", _filepath);
		return;
	}
	file.close();

	if (jsonData.empty())
	{
		GEAR_WARN(ErrorCode::CORE | ErrorCode::LOAD_FAILED, "%s is not valid.", _filepath);
	}

	std::string fileType = jsonData["fileType"];
	if (fileType.compare(fileTypeString) != 0)
	{
		GEAR_WARN(ErrorCode::CORE | ErrorCode::NOT_SUPPORTED, "%s is not valid.", _filepath);
	}
}

void gear::core::SaveJsonFile(const std::string& filepath, const std::string& fileExt, const std::string& fileTypeString, json& jsonData)
{
	jsonData["fileType"] = fileTypeString;

	std::string _filepath = filepath;
	if (_filepath.find(fileExt) == std::string::npos)
		_filepath += fileExt;

	const std::filesystem::path& folder = std::filesystem::path(_filepath).parent_path();
	if (!std::filesystem::exists(folder))
		std::filesystem::create_directory(folder);

	std::ofstream file(_filepath, std::ios::binary);
	if (!file.is_open())
	{
		GEAR_ASSERT(ErrorCode::SCENE | ErrorCode::NO_FILE, "Can not save to file: %s", _filepath);
		return;
	}
	else
	{
		file << std::setw(4) << jsonData;
	}
	file.close();
}
