#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace ui
	{
		class GEAR_API MenuBar
		{
			//enums/structs
		public:

			//Methods
		public:
			MenuBar();
			~MenuBar();

			void Draw();

		private:
			bool ShortcutPressed(const std::vector<uint32_t>& keycodes);

			void ProcessShortcuts();

			void DrawMenuFile();
			void DrawMenuSettings();
			void DrawMenuWindows();
			void DrawMenuHelp();

			void DrawItemNewProject();
			void DrawItemOpenProject();

			void DrawItemNewScene();
			void DrawItemOpenScene();
			void DrawItemSaveScene();
			void DrawItemSaveSceneAs();
			void DrawItemExit();

			void DrawItemAbout();
			void DrawItemGEARBOXOptions();
			void DrawItemRecompileRenderPipelineShaders();

			//Members
		private:
			typedef void(MenuBar::* PFN_PopupWindowFunction)();
			PFN_PopupWindowFunction m_PopupWindowFunction = nullptr;

			bool m_SaveToSaveAs = false;
		};
	}
}
