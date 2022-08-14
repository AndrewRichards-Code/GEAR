#include "gear_core_common.h"
#include "UI/Panels/RendererPropertiesPanel.h"
#include "UI/UIContext.h"
#include "UI/ComponentUI/ComponentUIs.h"

#include "Graphics/Rendering/Renderer.h"
#include "Graphics/Rendering/RenderGraph.h"

using namespace gear;
using namespace graphics::rendering;
using namespace scene;
using namespace ui;
using namespace panels;
using namespace componentui;

using namespace mars;

RendererPropertiesPanel::RendererPropertiesPanel()
{
	m_Type = Type::RENDERER_PROPERTIES;
}

RendererPropertiesPanel::~RendererPropertiesPanel()
{
}

void RendererPropertiesPanel::Draw()
{
	std::string id = UIContext::GetUIContext()->GetUniqueIDString("Renderer Properties", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		if (m_ViewportPanel)
		{
			Ref<Renderer>& renderer = m_ViewportPanel->GetCreateInfo().renderer;
			const RenderGraph& renderGraph = renderer->GetRenderGraph();
			
			auto GetResourceTypeString = [](const ResourceView& resourceView) -> std::string
			{
				if (resourceView.IsImageView())
					return "Image View";
				else if (resourceView.IsSampler())
					return "Sampler";
				else if (resourceView.IsBufferView())
					return "Buffer View";
				else if (resourceView.IsAccelerationStructure())
					return "Acceleration Structure";
				else
					return "";
			};

			size_t id = 1;
			ImGui::Text("Topologically Sorted Passes:");
			for (const auto& pass : renderGraph.GetTopologicallySortedPasses(renderer->GetFrameIndex()))
			{
				constexpr auto resourceStates = magic_enum::enum_names<Resource::State>();
				constexpr auto stages = magic_enum::enum_names<miru::base::Shader::StageBit>();

				const std::string& name = pass->GetName();
				if (DrawTreeNode(name, false, (void*)id++))
				{
					DrawStaticText("", "Inputs", 0.0f);
					for (const auto& resourceView : pass->GetInputResourceViews())
					{
						const std::string& stage = resourceView.GetStage() != (miru::base::Shader::StageBit)0 ? std::string(stages[static_cast<size_t>(log2((double)resourceView.GetStage()))]) : "";
						std::string name = resourceView.GetResource().GetName();
						name = GetResourceTypeString(resourceView) + ": " + name.substr(name.find_last_of(' ') + 1) + ": ";
						const std::string& state = std::string(resourceStates[static_cast<size_t>(resourceView.GetState())]);

						DrawStaticText("", name, 10.0f);
						DrawStaticText("", state, 30.0f);
						DrawStaticText("", stage, 30.0f);
					}

					DrawStaticText("", "Outputs", 0.0f);
					for (const auto& resourceView : pass->GetOutputResourceViews())
					{
						const std::string& stage = resourceView.GetStage() != (miru::base::Shader::StageBit)0 ? std::string(stages[static_cast<size_t>(log2((double)resourceView.GetStage()))]) : "";
						std::string name = resourceView.GetResource().GetName();
						name = GetResourceTypeString(resourceView) + ": " + name.substr(name.find_last_of(' ') + 1) + ": ";
						const std::string& state = std::string(resourceStates[static_cast<size_t>(resourceView.GetState())]);

						DrawStaticText("", name, 10.0f);
						DrawStaticText("", state, 30.0f);
						DrawStaticText("", stage, 30.0f);
					}
					EndDrawTreeNode();
				}
			}
			ImGui::Separator();
		}
	}
	ImGui::End();
}