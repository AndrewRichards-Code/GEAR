#include "gear_core_common.h"
#include "UI/ComponentUI/ModelComponentUI.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/UIContext.h"
#include "UI/Panels/ViewportPanel.h"
#include "UI/Panels/MaterialPanel.h"

#include "Animation/Animation.h"
#include "Graphics/Rendering/Renderer.h"
#include "Scene/Entity.h"

using namespace gear;
using namespace scene;
using namespace objects;
using namespace graphics;
using namespace rendering;

using namespace ui;
using namespace panels;
using namespace componentui;

using namespace mars;

void gear::ui::componentui::DrawModelComponentUI(Entity entity)
{
	Ref<Model>& model = entity.GetComponent<ModelComponent>().model;
	Model::CreateInfo& CI = model->m_CI;
	
	DrawInputText("Name", CI.debugName);
	DrawMeshUI(CI.pMesh);
	DrawFloat("Scaling X", CI.materialTextureScaling.x, 0.0f);
	DrawFloat("Scaling Y", CI.materialTextureScaling.y, 0.0f);
	DrawInputText("Render Pipeline", CI.renderPipelineName);

	Ref<ViewportPanel> viewportPanel = UIContext::GetUIContext()->GetEditorPanelsByType<ViewportPanel>()[0];
	if (viewportPanel)
	{
		Ref<Renderer>& renderer = viewportPanel->GetCreateInfo().renderer;
		RenderPipeline* renderPipeline = renderer->GetRenderPipelines()[CI.renderPipelineName].get();

		Ref<MaterialPanel> materialPanel = UIContext::GetUIContext()->GetEditorPanelsByType<MaterialPanel>()[0];
		if (materialPanel)
		{
			materialPanel->SetSelectedRenderPipline(renderPipeline);
		}

	}

	model->Update(entity.GetComponent<TransformComponent>());
}

void gear::ui::componentui::AddModelComponent(Entity entity, void* device)
{
	if (!entity.HasComponent<ModelComponent>())
	{
		Mesh::CreateInfo meshCI;
		meshCI.debugName = "Mesh-Cube";
		meshCI.device = device;
		meshCI.filepath = "res/obj/cube.fbx";
		Model::CreateInfo modelCI;
		modelCI.debugName = "Model-Cube";
		modelCI.device = device;
		modelCI.pMesh = CreateRef<Mesh>(&meshCI);
		modelCI.renderPipelineName = "PBROpaque";
		entity.AddComponent<ModelComponent>(&modelCI);

		Transform& transform = entity.GetComponent<TransformComponent>();
		transform.translation = float3(0, 0, 0);
		transform.orientation = Quaternion(1, 0, 0, 0);
		transform.scale = float3(1.0f, 1.0f, 1.0f);
	}
}

void gear::ui::componentui::DrawMeshUI(Ref<Mesh>& mesh)
{
	Mesh::CreateInfo& CI = mesh->m_CI;

	if (DrawTreeNode("Mesh", false))
	{
		DrawInputText("Filepath", CI.filepath);
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".fbx");
			if (payload)
			{
				std::string filepath = (char*)payload->Data;
				if (std::filesystem::exists(filepath))
				{
					CI.filepath = filepath;
				}
			}
		}
		auto& data = mesh->GetModelData();
		
		size_t id = 1;
		if (DrawTreeNode("Sub Meshes: " + std::to_string(data.meshes.size()), false))
		{
			for (size_t i = 0; i < data.meshes.size(); i++)
			{
				auto& subMesh = data.meshes[i];
				if (DrawTreeNode("Sub Mesh: " + std::to_string(i), false, (void*)id++))
				{
					DrawStaticText("Name", subMesh.meshName);
					if (ImGui::Button("View Material"))
					{
						Ref<MaterialPanel> materialPanel = UIContext::GetUIContext()->GetEditorPanelsByType<MaterialPanel>()[0];
						if (materialPanel)
						{
							materialPanel->SetSelectedMaterial(mesh->GetMaterial(i));
						}
					}
					DrawStaticNumber("Vertices", subMesh.vertices.size());
					DrawStaticNumber("Indices", subMesh.indices.size());
					DrawStaticNumber("Bones", subMesh.bones.size());
					EndDrawTreeNode();
				}
			}
			EndDrawTreeNode();
		}
		if (DrawTreeNode("Animations: " + std::to_string(data.animations.size()), false))
		{
			for (size_t i = 0; i < data.animations.size(); i++)
			{
				const auto& animation = data.animations[i];
				if (DrawTreeNode("Animation: " + std::to_string(i), false, (void*)id++))
				{
					DrawEnum("Type", animation.sequenceType);
					DrawStaticNumber("Duration", animation.duration);
					DrawStaticNumber("Frames Per Second", animation.framesPerSecond);

					for (size_t j = 0; j < animation.nodeAnimations.size(); j++)
					{
						const auto& nodeAnimation = animation.nodeAnimations[j];
						if (DrawTreeNode("Node Animation: " + std::to_string(j), false, (void*)id++))
						{
							DrawStaticText("Name", nodeAnimation.name);
							DrawEnum("Type", nodeAnimation.type);
							DrawStaticNumber("Duration", nodeAnimation.keyframes.size());
							EndDrawTreeNode();
						}
					}
					EndDrawTreeNode();
				}
			}
			EndDrawTreeNode();
		}
	
		mesh->Update();

		EndDrawTreeNode();
	}
}
