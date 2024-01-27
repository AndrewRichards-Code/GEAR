#pragma once
#include "UI/Panels/BasePanel.h"

namespace gear
{
	namespace graphics::rendering
	{
		class Renderer;
	}
	namespace objects
	{
		class Camera;
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
					Ref<graphics::rendering::Renderer> renderer;
				};
				enum class TransformationType : uint8_t
				{
					TRANSLATION,
					ROTATION,
					SCALE
				};

				//Methods
			public:
				ViewportPanel(CreateInfo* pCreateInfo);
				~ViewportPanel();

				void Draw() override;

			private:
				void DrawOverlay(const std::string& parentPanelID);
				void DrawGuizmos();

				void UpdateCameraTransform();
				
				mars::float2 GetMousePositionInViewport();
				float GetMouseScrollWheel();

			public:
				inline CreateInfo& GetCreateInfo() { return m_CI; }

				//Members
			private:
				CreateInfo m_CI;

				static uint32_t s_ViewportPanelCount;
				uint32_t m_ViewID; //0 is invalid;

				void* m_ImageID = nullptr;
				mars::float2 m_CurrentSize;

				//Camera Control
				float m_Roll = 0.0f;
				float m_Pitch = 0.0f;
				float m_Yaw = 0.0f;
				float m_CameraSpeed = 1.0f;
				mars::float2 m_InitialMousePosition;
				float m_InitalMouseScrollWheel = 0.0f;
				bool m_GuizmoActive = false;
				bool m_DebugRendering = true;
				
				TransformationType m_TransformType = TransformationType::TRANSLATION;
				bool m_TransformSnapping = false;
				float m_TransformSnappingValues[3] = { 0.1f, 1.0f, 0.1f };
			};
		}
	}
}
