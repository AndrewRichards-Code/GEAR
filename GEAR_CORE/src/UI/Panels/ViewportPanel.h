#pragma once
#include "UI/Panels/BasePanel.h"

namespace gear
{
	namespace graphics
	{
		class Renderer;
	}
	namespace ui
	{
		namespace panels
		{
			class GEAR_API ViewportPanel final : public Panel
			{
				//enums/structs
			public:
				struct CreateInfo
				{
					Ref<graphics::Renderer> renderer;
				};

				//Methods
			public:
				ViewportPanel(CreateInfo* pCreateInfo);
				~ViewportPanel();

				void Draw() override;

			private:
				void UpdateCameraTransform();
				mars::float2 GetMousePositionInViewport();

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
				mars::float2 m_InitialMousePosition;
			};
		}
	}
}
