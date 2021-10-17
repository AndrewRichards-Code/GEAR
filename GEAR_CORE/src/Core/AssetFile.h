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
	class AssetFile
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

		static std::string FileDialog_Open(const Ref<graphics::Window>& window);
		static std::string FileDialog_Save(const Ref<graphics::Window>& window);

		void Load();
		void Save();

		bool Contains(Type type);

		//Members
		CreateInfo m_CI;
		nlohmann::json m_AssetData;
	};
}
}
