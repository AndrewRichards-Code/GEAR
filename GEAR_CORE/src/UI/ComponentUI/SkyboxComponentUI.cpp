#include "gear_core_common.h"
#include "SkyboxComponentUI.h"
#include "Scene/Entity.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace ui;
using namespace componentui;

using namespace mars;

void gear::ui::componentui::DrawSkyboxComponentUI(gear::scene::Entity entity)
{
	Ref<Skybox>& skybox = entity.GetComponent<SkyboxComponent>().skybox;
	Skybox::CreateInfo& CI = skybox->m_CI;

	bool hdr = true;
	DrawCheckbox("HDR", hdr);
	if (hdr)
	{
		DrawInputText("Filepath", CI.filepath);
	}
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".hdr");
		if (payload)
		{
			std::string filepath = (char*)payload->Data;
			if (std::filesystem::exists(filepath))
				CI.filepath = filepath;
		}
	}
	DrawUint32("Cubemap Size", CI.generatedCubemapSize, 16, 2048, true);
	
	
	if (ImGui::Button("Generate"))
		skybox->m_Generated = false;
	
	skybox->Update(entity.GetComponent<TransformComponent>());
}

void gear::ui::componentui::AddSkyboxComponent(gear::scene::Entity entity, void* device)
{
	if (!entity.HasComponent<SkyboxComponent>())
	{
		Skybox::CreateInfo skyboxCI;
		skyboxCI.debugName = "Skybox-HDR";
		skyboxCI.device = device;
		skyboxCI.filepath = "res/img/kloppenheim_06_2k.hdr";
		skyboxCI.generatedCubemapSize = 1024;
		entity.AddComponent<SkyboxComponent>(&skyboxCI);

		Transform& transform = entity.GetComponent<TransformComponent>();
		transform.translation = float3(0, 0, 0);
		transform.orientation = Quaternion(1, 0, 0, 0);
		transform.scale = float3(500.0f, 500.0f, 500.0f);
	}
}
