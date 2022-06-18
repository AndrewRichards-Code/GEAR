#include "gear_core_common.h"
#include "Core/JsonFileHelper.h"

using namespace nlohmann;

void gear::core::LoadJsonFile(std::string& filepath, const std::string& fileExt, const std::string& fileTypeString, json& jsonData)
{
	if (filepath.find(fileExt) == std::string::npos)
		filepath += fileExt;

	std::ifstream file(filepath, std::ios::binary);
	if (file.is_open())
	{
		file >> jsonData;
	}
	else
	{
		GEAR_ASSERT(ErrorCode::CORE | ErrorCode::NO_FILE, "Unable to open %s.", filepath);
		return;
	}
	file.close();

	if (jsonData.empty())
	{
		GEAR_WARN(ErrorCode::CORE | ErrorCode::LOAD_FAILED, "%s is not valid.", filepath);
	}

	std::string fileType = jsonData["fileType"];
	if (fileType.compare(fileTypeString) != 0)
	{
		GEAR_WARN(ErrorCode::CORE | ErrorCode::NOT_SUPPORTED, "%s is not valid.", filepath);
	}
}

void gear::core::SaveJsonFile(std::string& filepath, const std::string& fileExt, const std::string& fileTypeString, json& jsonData)
{
	jsonData["fileType"] = fileTypeString;

	if (filepath.find(fileExt) == std::string::npos)
		filepath += fileExt;

	std::filesystem::path& folder = std::filesystem::path(filepath).parent_path();
	if (!std::filesystem::exists(folder))
		std::filesystem::create_directory(folder);

	std::ofstream file(filepath, std::ios::binary);
	if (!file.is_open())
	{
		GEAR_ASSERT(ErrorCode::SCENE | ErrorCode::NO_FILE, "Can not save to file: %s", filepath);
		return;
	}
	else
	{
		file << std::setw(4) << jsonData;
	}
	file.close();
}
