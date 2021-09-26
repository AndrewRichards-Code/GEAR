#pragma once
#include "Panel.h"
#include "UIContext.h"

namespace gearbox
{
	namespace panels
	{
		class ContentEditorPanel final : public Panel
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
				COMPRESSED
			};

			struct CreateInfo
			{
				Ref<imgui::UIContext>	uiContext;
				std::filesystem::path	currentFilepath;
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
			size_t fileSize = 0;
			std::string output;
			
		};
	}
}