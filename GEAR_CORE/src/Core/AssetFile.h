#pragma once
#include "gear_core_common.h"

namespace gear
{
namespace graphics
{
	class Window;
}

namespace core
{
	class GEAR_API AssetFile
	{
		//enum/structs
	public:
		enum class Type : uint32_t
		{
			NONE,
			MATERIAL
		};

		struct CreateInfo
		{
			std::string filepath;
		};

		//Methods
		AssetFile(CreateInfo* pCreateInfo);
		AssetFile(const std::string& filepath);
		~AssetFile();

		static std::string FileDialog_Open();
		static std::string FileDialog_Save();

		void Load();
		void Save();

		bool Contains(Type type);

		//Members
		CreateInfo m_CI;
		nlohmann::json m_AssetData;
	};
}
}
