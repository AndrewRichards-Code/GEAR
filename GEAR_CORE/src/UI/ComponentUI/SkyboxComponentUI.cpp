#include "gear_core_common.h"
#include "UI/ComponentUI/SkyboxComponentUI.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/UIContext.h"
#include "Scene/Entity.h"
#include "Asset/EditorAssetManager.h"

using namespace gear;
using namespace scene;
using namespace objects;
using namespace project;

using namespace ui;
using namespace componentui;

using namespace mars;

void gear::ui::componentui::DrawSkyboxComponentUI(Entity entity)
{
	Ref<Skybox>& skybox = entity.GetComponent<SkyboxComponent>().skybox;
	Skybox::CreateInfo& CI = skybox->m_CI;

	Ref<asset::EditorAssetManager> editorAssetManager = UIContext::GetUIContext()->GetEditorAssetManager();

	bool hdr = true;
	DrawCheckbox("HDR", hdr);
	if (hdr)
	{
		DrawStaticText("Filepath", editorAssetManager->GetFilepath(CI.textureData->handle).generic_string());
	}
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".hdr");
		if (payload)
		{
			std::filesystem::path filepath = (char*)payload->Data;
			if (std::filesystem::exists(filepath))
				CI.textureData = editorAssetManager->Import<asset::ImageAssetDataBuffer>(asset::Asset::Type::EXTERNAL_FILE, filepath);
		}
	}
	DrawUint32("Cubemap Size", CI.generatedCubemapSize, 16, 2048, true);
	
	
	if (ImGui::Button("Generate"))
		skybox->m_Generated = false;
	
	skybox->Update(entity.GetComponent<TransformComponent>());
}

void gear::ui::componentui::AddSkyboxComponent(Entity entity, void* device)
{
	if (!entity.HasComponent<SkyboxComponent>())
	{
		Ref<asset::EditorAssetManager> editorAssetManager = UIContext::GetUIContext()->GetEditorAssetManager();

		Skybox::CreateInfo skyboxCI;
		skyboxCI.debugName = "Skybox-HDR";
		skyboxCI.device = device;
		skyboxCI.textureData = editorAssetManager->Import<asset::ImageAssetDataBuffer>(asset::Asset::Type::EXTERNAL_FILE, "res/img/kloppenheim_06_2k.hdr");
		skyboxCI.generatedCubemapSize = 1024;
		entity.AddComponent<SkyboxComponent>(&skyboxCI);

		Transform& transform = entity.GetComponent<TransformComponent>();
		transform.translation = float3(0, 0, 0);
		transform.orientation = Quaternion(1, 0, 0, 0);
		transform.scale = float3(500.0f, 500.0f, 500.0f);
	}
}
