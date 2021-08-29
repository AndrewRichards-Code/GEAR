#pragma once
#include "Panel.h"
#include "UIContext.h"

namespace gearbox
{
	namespace panels
	{
		class ViewportPanel final : public Panel
		{
			//enums/structs
		public:
			struct CreateInfo
			{
				Ref<gear::graphics::Renderer>	renderer;
				Ref<imgui::UIContext>			uiContext;
			};

			//Methods
		public:
			ViewportPanel(CreateInfo* pCreateInfo);
			~ViewportPanel();

			void Draw() override;

		private:
			void UpdateCameraTransform();
			mars::Vec2 GetMousePositionInViewport();

		public:
			inline CreateInfo& GetCreateInfo() { return m_CI; }

			//Members
		private:
			CreateInfo m_CI;

			static uint32_t s_ViewportPanelCount;
			uint32_t m_ViewID; //0 is invalid;
			
			ImTextureID m_ImageID = 0;
			ImVec2 m_CurrentSize;
			
			//Camera Control
			float m_Roll = 0;
			float m_Pitch = 0;
			float m_Yaw = 0;
			mars::Vec2 m_InitialMousePosition;
		};
	}
}
