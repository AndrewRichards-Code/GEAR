#include "gearbox_common.h"
#include "SkyboxComponentUI.h"

#include "ARC/src/FileSystemHelpers.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace gearbox;
using namespace componentui;

using namespace mars;

void gearbox::componentui::DrawSkyboxComponentUI(gear::scene::Entity entity)
{
	Ref<Skybox>& skybox = entity.GetComponent<SkyboxComponent>().skybox;
	Skybox::CreateInfo& CI = skybox->m_CI;

	bool hdr = true;
	DrawCheckbox("HDR", hdr);
	if (hdr)
	{
		if (DrawInputText("Filepath", CI.filepaths[0]))
		{
			if(arc::FileExist(CI.filepaths[0]) && CI.filepaths[0].find(".hdr") != std::string::npos)
				skybox->m_Reload = true;
		}
	}
	else
	{
		for (auto& filepath : CI.filepaths)
		{
			DrawInputText("Filepath", filepath);
		}
	}
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".hdr");
		if (payload)
		{
			std::string filepath = (char*)payload->Data;
			if (arc::FileExist(filepath))
			{
				CI.filepaths[0] = filepath;
				skybox->m_Reload = true;
			}
		}
	}


	DrawUint32("Cubemap Size", CI.generatedCubemapSize, 16, 2048, true);
	DrawFloat("Exposure", CI.exposure, 0.01f, 100.0f);
	DrawDropDownMenu("Colour Space", { "CieXYZ", "sRGB", "Rec709", "Rec2020" }, CI.gammaSpace);
	
	if (ImGui::Button("Generate"))
		skybox->m_Generated = false;
	
	CI.transform = entity.GetComponent<TransformComponent>().transform;
	
	skybox->Update();
}

void gearbox::componentui::AddSkyboxComponent(gear::scene::Entity entity, void* device)
{
	if (!entity.HasComponent<SkyboxComponent>())
	{
		Skybox::CreateInfo skyboxCI;
		skyboxCI.debugName = "Skybox-HDR";
		skyboxCI.device = device;
		skyboxCI.filepaths = { "../GEAR_TEST/res/img/kloppenheim_06_2k.hdr" };
		skyboxCI.generatedCubemapSize = 1024;
		skyboxCI.transform.translation = Vec3(0, 0, 0);
		skyboxCI.transform.orientation = Quat(1, 0, 0, 0);
		skyboxCI.transform.scale = Vec3(500.0f, 500.0f, 500.0f);
		entity.AddComponent<SkyboxComponent>(&skyboxCI);

		entity.GetComponent<TransformComponent>().transform = skyboxCI.transform;
	}
}
