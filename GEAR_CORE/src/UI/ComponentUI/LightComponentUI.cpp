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
	UIContext* uiContext = UIContext::GetUIContext();
	
	size_t id = 1;
	if (DrawTreeNode("Probe", true, (void*)id++))
	{
		Probe::CreateInfo& CI = probe->m_CI;

		DrawDropDownMenu("Direction Type", CI.directionType);
		DrawDropDownMenu("Capture Type", CI.captureType);
		uint32_t minSize = 16, maxSize = 2048;
		DrawUint32("Image Size", CI.imageSize, minSize, maxSize, true, 100.0f, 0.001f);
		if (CI.directionType == Probe::DirectionType::MONO)
		{
			DrawDropDownMenu("Type", CI.projectionType);
			if (CI.projectionType == Camera::ProjectionType::PERSPECTIVE)
			{
				double fovDeg = RadToDeg(CI.perspectiveHorizonalFOV);
				DrawDouble("FOV", fovDeg, 0.0, 180.0);
				CI.perspectiveHorizonalFOV = DegToRad(fovDeg);
			}
		}
		DrawFloat("Near", CI.zNear, 0.0, CI.zFar);
		DrawFloat("Far", CI.zFar, CI.zNear);
		if (CI.captureType == Probe::CaptureType::SHADOW)
		{
			DrawStaticText("Shadows", "");
			if (CI.projectionType == Camera::ProjectionType::ORTHOGRAPHIC)
			{
				DrawUint32("Cascades", CI.shadowCascades, 1, Probe::MaxShadowCascades, false, 100.0f, 0.1f);
				DrawFloat("Distance 0", CI.shadowCascadeDistances[0], CI.zNear, CI.shadowCascadeDistances[1]);
				DrawFloat("Distance 1", CI.shadowCascadeDistances[1], CI.shadowCascadeDistances[0], CI.shadowCascadeDistances[2]);
				DrawFloat("Distance 2", CI.shadowCascadeDistances[2], CI.shadowCascadeDistances[1], CI.shadowCascadeDistances[3]);
				DrawFloat("Distance 3", CI.shadowCascadeDistances[3], CI.shadowCascadeDistances[2]);
				
				DrawCheckbox("Auto Distance", CI.calculateShadowCascadeDistances);
				if (CI.calculateShadowCascadeDistances)
				{
					DrawFloat("Split Lambda", CI.shadowCascadeSplitLambda, 0.1f, 1.0f, 100.0f, 0.001);
				}
			}

			if (DrawTreeNode("Shadow Map Debug View", true, (void*)id++))
			{
				using namespace miru::base;
				using namespace graphics;

				probe->m_RenderDebugView = true;
				Ref<Uniformbuffer<UniformBufferStructures::DebugProbeInfo>>& debugProbeInfo = DebugRender::GetDebugProbeInfo();

				//Debug Texture
				if (probe->m_DebugTexture)
				{
					const ImageViewRef& debugImageView = probe->m_DebugTexture->GetImageView();
					const Image::CreateInfo& debugImageCI = debugImageView->GetCreateInfo().image->GetCreateInfo();
					float imageRatio = float(debugImageCI.width) / float(debugImageCI.height);
					float width = std::max(ImGui::GetContentRegionMax().x - 100.0f, 1.0f);
					float height = width * imageRatio;

					ImageViewRef* debugImageViews = probe->m_DebugImageViews;
					for (uint32_t i = 0; i < CI.shadowCascades; i++)
					{
						ImageView::CreateInfo debugImageViewCI = debugImageView->GetCreateInfo();
						debugImageViewCI.debugName += " " + std::to_string(i);
						debugImageViewCI.viewType = Image::Type::TYPE_2D;
						debugImageViewCI.subresourceRange.baseArrayLayer = i;
						debugImageViewCI.subresourceRange.arrayLayerCount = 1;

						if (!debugImageViews[i])
						{
							debugImageViews[i] = ImageView::Create(&debugImageViewCI);
						}
						if (debugImageViews[i]->GetCreateInfo().image != debugImageView->GetCreateInfo().image)
						{
							debugImageViews[i] = ImageView::Create(&debugImageViewCI);
						}
					}

					static uint32_t idx = 0;
					DrawDropDownMenu("Shadow Cascade", { "0", "1", "2", "3" }, idx);
					idx = std::min(idx, (CI.shadowCascades - 1));
					const ImTextureID& id = uiContext->AddTextureID(debugImageViews[idx]);
					ImGui::Image(id, ImVec2(width, height));

					debugProbeInfo->proj = probe->GetUB()->proj[idx];
					debugProbeInfo->view = probe->GetUB()->view[idx];
				}

				//Debug Texture Camera Controls
				static mars::float2 m_InitialMousePosition;
				auto GetMousePositionInViewport = [&]() -> float2
					{
						double mousePosition_x, mousePosition_y;
						uiContext->GetWindow()->GetMousePosition(mousePosition_x, mousePosition_y);

						return float2((float)mousePosition_x, (float)mousePosition_y);
					};

				if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
				{
					if (probe->m_CI.directionType == Probe::DirectionType::OMNI)
					{
						Ref<Camera>& debugCamera = DebugRender::GetCamera();
						static Transform transform = Transform();
						static float m_Roll = 0;
						static float m_Pitch = 0;
						static float m_Yaw = 0;

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
				}
				else
				{
					m_InitialMousePosition = GetMousePositionInViewport();
				}

				DrawCheckbox("Show Colour Cubemap", (bool&)debugProbeInfo->showColourCubemap);
				DrawFloat("Min Depth", debugProbeInfo->minDepth, 0.0f, debugProbeInfo->maxDepth, DefaultWidth, 1e-3f, "%.3f");
				DrawFloat("Max Depth", debugProbeInfo->maxDepth, debugProbeInfo->minDepth, 1.0f, DefaultWidth, 1e-3f, "%.3f");

				EndDrawTreeNode();
			}
			else
			{
				probe->m_RenderDebugView = false;
			}
		}

		EndDrawTreeNode();
	}
}
