#include "gear_core_common.h"
#include "UI/ComponentUI/LightComponentUI.h"
#include "UI/UIContext.h"

#include "Scene/Entity.h"
#include "Graphics/DebugRender.h"
#include "Graphics/Window.h"
#include "Objects/Light.h"
#include "Objects/Probe.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace ui;
using namespace panels;
using namespace componentui;

using namespace mars;

void gear::ui::componentui::DrawLightComponentUI(Entity entity)
{
	Ref<Light>& light = entity.GetComponent<LightComponent>().light;
	Light::CreateInfo& CI = light->m_CI;

	if (DrawTreeNode("Light"))
	{
		DrawDropDownMenu("Type", CI.type);
		DrawColourPicker4("Colour", CI.colour);
		if (CI.type == Light::Type::SPOT)
		{
			float inner = RadToDeg(CI.spotInnerAngle);
			float outer = RadToDeg(CI.spotOuterAngle);
			DrawFloat("Spot Inner Angle", inner, 0.0, outer);
			DrawFloat("Spot Outer Angle", outer, inner);
			CI.spotInnerAngle = DegToRad(inner);
			CI.spotOuterAngle = DegToRad(outer);
		}

		EndDrawTreeNode();
	}

	DrawProbeComponentUI(light->GetProbe());

	light->Update(entity.GetComponent<TransformComponent>());
}

void gear::ui::componentui::AddLightComponent(Entity entity, void* device)
{
	if (!entity.HasComponent<CameraComponent>())
	{
		Light::CreateInfo lightCI;
		lightCI.debugName = entity.GetComponent<NameComponent>();
		lightCI.device = device;
		lightCI.type = Light::Type::DIRECTIONAL;
		lightCI.colour = float4(1.0f, 1.0f, 1.0f, 1.0f);
		lightCI.spotInnerAngle = DegToRad(45.0f);
		lightCI.spotOuterAngle = DegToRad(45.0f);
		entity.AddComponent<LightComponent>(&lightCI);
	}
}

void gear::ui::componentui::DrawProbeComponentUI(Ref<Probe> probe)
{
	if (DrawTreeNode("Probe"))
	{
		Probe::CreateInfo& CI = probe->m_CI;

		DrawDropDownMenu("Direction Type", CI.directionType);
		DrawDropDownMenu("Capture Type", CI.captureType);
		if (CI.directionType == Probe::DirectionType::MONO)
		{
			DrawUint32("Image Width", CI.imageWidth, 1, 16384);
			DrawUint32("Image Height", CI.imageHeight, 1, 16384);
		}
		else
		{
			DrawUint32("Image Size", CI.imageWidth, 1, 16384, true);
		}
		if (CI.directionType == Probe::DirectionType::MONO)
		{
			DrawDropDownMenu("Type", CI.projectionType);
			if (CI.projectionType == Camera::ProjectionType::PERSPECTIVE)
			{
				double fovDeg = RadToDeg(CI.perspectiveHorizonalFOV);
				DrawDouble("FOV", fovDeg, 0.0, 180.0);
				CI.perspectiveHorizonalFOV = DegToRad(fovDeg);
			}
			else
			{
				DrawFloat("Orthographic Size", CI.orthographicScale, 1.0f);
			}
		}
		DrawFloat("Near", CI.zNear, 0.0, CI.zFar);
		DrawFloat("Far", CI.zFar, CI.zNear);

		if (DrawTreeNode("Shadow Map Debug View"))
		{
			probe->m_RenderDebugView = true;
			if (probe->m_DebugTexture)
			{
				const miru::base::ImageViewRef& debugImageView = probe->m_DebugTexture->GetImageView();
				const miru::base::Image::CreateInfo& debugImageCI = debugImageView->GetCreateInfo().image->GetCreateInfo();
				float imageRatio = float(debugImageCI.width) / float(debugImageCI.height);
				float width = std::max(ImGui::GetContentRegionMax().x - 100.0f, 1.0f);
				float height = width * imageRatio;
				const ImTextureID& id = GetTextureID(debugImageView, UIContext::GetUIContext(), false);
				ImGui::Image(id, ImVec2(width, height));

				Ref<Camera>& debugCamera = graphics::DebugRender::GetCamera();
				static Transform& transform = Transform();
				static float m_Roll = 0;
				static float m_Pitch = 0;
				static float m_Yaw = 0;
				static mars::float2 m_InitialMousePosition;

				auto GetMousePositionInViewport = [&]() -> float2
				{
					double mousePosition_x, mousePosition_y;
					UIContext::GetUIContext()->GetWindow()->GetMousePosition(mousePosition_x, mousePosition_y);

					return float2((float)mousePosition_x, (float)mousePosition_y);
				};

				//Stop camera snapping back when new input is detected
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
				{
					m_InitialMousePosition = GetMousePositionInViewport();
				}

				//Main update
				if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Right))
				{
					const float2& mouse = GetMousePositionInViewport();
					float2 delta = (mouse - m_InitialMousePosition) * 0.003f;
					m_InitialMousePosition = mouse;

					m_Yaw += delta.x;
					m_Pitch += delta.y;

					transform.orientation = Quaternion::FromEulerAngles(float3(m_Pitch, m_Yaw, m_Roll));
				}

				debugCamera->Update(transform);

			}
			else
			{
				const miru::base::ImageViewRef& debugImageView = probe->m_DepthTexture->GetImageView();
				const miru::base::Image::CreateInfo& debugImageCI = debugImageView->GetCreateInfo().image->GetCreateInfo();
				float imageRatio = float(debugImageCI.width) / float(debugImageCI.height);
				float width = std::max(ImGui::GetContentRegionMax().x - 100.0f, 1.0f);
				float height = width * imageRatio;
				const ImTextureID& id = GetTextureID(debugImageView, UIContext::GetUIContext(), false);
				ImGui::Image(id, ImVec2(width, height));
			}
		
			EndDrawTreeNode();
		}
		else
		{
			probe->m_RenderDebugView = false;
		}

		EndDrawTreeNode();
	}
}
