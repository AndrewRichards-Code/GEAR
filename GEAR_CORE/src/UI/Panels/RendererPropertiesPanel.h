#pragma once
#include "UI/Panels/BasePanel.h"
#include "UI/Panels/ViewportPanel.h"

namespace gear
{
	namespace ui
	{
		namespace panels
		{
			class GEAR_API RendererPropertiesPanel final : public Panel
			{
				//enums/structs
			public:

				//Methods
			public:
				RendererPropertiesPanel();
				~RendererPropertiesPanel();

				void Draw() override;

				inline void SetViewportPanel(const Ref<ViewportPanel>& viewportPanel) { m_ViewportPanel = viewportPanel; }

				//Members
			private:
				Ref<ViewportPanel> m_ViewportPanel;
			};
		}
	}
}
