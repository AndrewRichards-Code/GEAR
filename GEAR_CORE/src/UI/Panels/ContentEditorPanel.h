#pragma once
#include "UI/UIContext.h"
#include "Objects/Material.h"

namespace gear
{
namespace ui
{
namespace panels
{
	class GEAR_API ContentEditorPanel final : public Panel
	{
		//enums/structs
	public:
		enum class ContentType : uint32_t
		{
			UNKNOWN,
			TEXT,
			IMAGE,
			VIDEO,
			AUDIO,
			MODEL,
			FONT,
			LIBRARY_STATIC,
			LIBRARY_DYNAMIC,
			LIBRARY_SYMBOLS,
			SCRIPT,
			BINARY,
			COMPRESSED,

			GEAR_SCENE,
			GEAR_ASSET,
			GEAR_RENDER_PIPELINE
		};

		struct CreateInfo
		{
			std::filesystem::path	currentFilepathFull;
			std::string				filepathExt;
		};

		//Methods
	public:
		ContentEditorPanel(CreateInfo* pCreateInfo);
		~ContentEditorPanel();

		void Draw() override;

	public:
		inline CreateInfo& GetCreateInfo() { return m_CI; }
		ContentType GetContextTypeFromExtension(const std::string& fileExtension);

		//Members
	private:
		CreateInfo m_CI;

		bool open = false;
		bool close = false;
		bool read = true;
		std::string output;

		Ref<gear::objects::Material> material;
		
	};
}
}
}