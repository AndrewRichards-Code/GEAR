#include "gear_core_common.h"
#include "UI/Panels/RendererPropertiesPanel.h"
#include "UI/UIContext.h"
#include "UI/ComponentUI/ComponentUIs.h"

#include "Graphics/Rendering/Renderer.h"
#include "Graphics/Rendering/RenderGraph.h"
#include "Core/ParseStack.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
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

			std::stack<std::string> scopeStack;
			std::stack<bool> scopeStackOpen;
			size_t id = 1;
			ImGui::Text("Topologically Sorted Passes:");
			for (const auto& pass : renderGraph.GetTopologicallySortedPasses(renderer->GetFrameIndex()))
			{
				if (scopeStack != pass->GetScopeStack())
				{
					core::ResolveStacks<std::string>(scopeStack, pass->GetScopeStack(),
						[&]()
						{
							if (scopeStackOpen.top())
								EndDrawTreeNode();
							scopeStackOpen.pop();
						},
						[&](const std::string& scopeName)
						{
							bool open = false;
							if (scopeStackOpen.empty() || scopeStackOpen.top())
								open = DrawTreeNode(scopeName, false, (void*)id++);
							scopeStackOpen.push(open);
						});
				}

				if (scopeStackOpen.top())
				{
					using namespace miru::base;

					constexpr auto resourceStates = magic_enum::enum_names<Resource::State>();
					constexpr auto pipelineStages = magic_enum::enum_names<PipelineStageBit>();

					const std::string& name = pass->GetName();
					if (DrawTreeNode(name, false, (void*)id++))
					{
						DrawStaticText("", "Inputs:", 0.0f);
						for (const auto& resourceView : pass->GetInputResourceViews())
						{
							const std::string& pipelineStage = PipelineStageBitToString(resourceView.GetPipelineStageBit());
							std::string name = resourceView.GetResource().GetName();
							name = GetResourceTypeString(resourceView) + ": " + name.substr(name.find_last_of(' ') + 1) + ": ";
							const std::string& state = std::string(resourceStates[static_cast<size_t>(resourceView.GetState())]);

							const std::string& message = name + "[" + state + "] [" + pipelineStage + "]";
							DrawStaticText("", message, 20.0f);
						}

						DrawStaticText("", "Outputs:", 0.0f);
						for (const auto& resourceView : pass->GetOutputResourceViews())
						{
							const std::string& pipelineStage = PipelineStageBitToString(resourceView.GetPipelineStageBit());
							std::string name = resourceView.GetResource().GetName();
							name = GetResourceTypeString(resourceView) + ": " + name.substr(name.find_last_of(' ') + 1) + ": ";
							const std::string& state = std::string(resourceStates[static_cast<size_t>(resourceView.GetState())]);

							const std::string& message = name + "[" + state + "] [" + pipelineStage + "]";
							DrawStaticText("", message, 20.0f);
						}
						EndDrawTreeNode();
					}
				}
			}

			core::ClearStack(scopeStack, [&]()
				{
					if (scopeStackOpen.top())
						EndDrawTreeNode();
					scopeStackOpen.pop();
				});
			ImGui::Separator();

			DrawPostProcessingUI();
		}
	}
	ImGui::End();
}

void RendererPropertiesPanel::DrawPostProcessingUI()
{
	if (!m_ViewportPanel)
		return;

	Ref<Renderer>& renderer = m_ViewportPanel->GetCreateInfo().renderer;
	Renderer::PostProcessingInfo& postProcessingInfo = renderer->GetPostProcessingInfo();

	if (DrawTreeNode("Post Processing", true))
	{
		//Bloom
		if (DrawTreeNode("Bloom", true))
		{
			Ref<Uniformbuffer<UniformBufferStructures::BloomInfo>>& bloomInfoUB = postProcessingInfo.bloom.UB;

			if (bloomInfoUB)
			{
				DrawFloat("Threshold", bloomInfoUB->threshold, 0.0f);
				DrawFloat("Upsample Scale", bloomInfoUB->upsampleScale, 1.0f);
			}
			EndDrawTreeNode();
		}
		if (DrawTreeNode("HDR Mapping", true))
		{
			Ref<Uniformbuffer<UniformBufferStructures::HDRInfo>>& hdrSettingsUB = postProcessingInfo.hdrSettings.UB;
			if (hdrSettingsUB)
			{
				DrawFloat("Exposure", hdrSettingsUB->exposure, 0.01f, 100.0f);
				DrawDropDownMenu("Colour Space", (ColourSpace&)hdrSettingsUB->gammaSpace);
			}
			EndDrawTreeNode();
		}

		EndDrawTreeNode();
	}
}

std::string RendererPropertiesPanel::PipelineStageBitToString(miru::base::PipelineStageBit pipelingStages)
{
	using namespace miru::base;

	auto GetSinglePipelineStageString = [](PipelineStageBit pipelingStage) -> std::string
		{
			switch (pipelingStage)
			{
			case PipelineStageBit::NONE_BIT:
				return "NONE_BIT";
			case PipelineStageBit::TOP_OF_PIPE_BIT:
				return "TOP_OF_PIPE_BIT";
			case PipelineStageBit::DRAW_INDIRECT_BIT:
				return "DRAW_INDIRECT_BIT";
			case PipelineStageBit::VERTEX_INPUT_BIT:
				return "VERTEX_INPUT_BIT";
			case PipelineStageBit::VERTEX_SHADER_BIT:
				return "VERTEX_SHADER_BIT";
			case PipelineStageBit::TESSELLATION_CONTROL_SHADER_BIT:
				return "TESSELLATION_CONTROL_SHADER_BIT";
			case PipelineStageBit::TESSELLATION_EVALUATION_SHADER_BIT:
				return "TESSELLATION_EVALUATION_SHADER_BIT";
			case PipelineStageBit::GEOMETRY_SHADER_BIT:
				return "GEOMETRY_SHADER_BIT";
			case PipelineStageBit::FRAGMENT_SHADER_BIT:
				return "FRAGMENT_SHADER_BIT";
			case PipelineStageBit::EARLY_FRAGMENT_TESTS_BIT:
				return "EARLY_FRAGMENT_TESTS_BIT";
			case PipelineStageBit::LATE_FRAGMENT_TESTS_BIT:
				return "LATE_FRAGMENT_TESTS_BIT";
			case PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT:
				return "COLOUR_ATTACHMENT_OUTPUT_BIT";
			case PipelineStageBit::COMPUTE_SHADER_BIT:
				return "COMPUTE_SHADER_BIT";
			case PipelineStageBit::TRANSFER_BIT:
				return "TRANSFER_BIT";
			case PipelineStageBit::BOTTOM_OF_PIPE_BIT:
				return "BOTTOM_OF_PIPE_BIT";
			case PipelineStageBit::HOST_BIT:
				return "HOST_BIT";
			case PipelineStageBit::ALL_GRAPHICS_BIT:
				return "ALL_GRAPHICS_BIT";
			case PipelineStageBit::ALL_COMMANDS_BIT:
				return "ALL_COMMANDS_BIT";
			case PipelineStageBit::TRANSFORM_FEEDBACK_BIT:
				return "TRANSFORM_FEEDBACK_BIT";
			case PipelineStageBit::CONDITIONAL_RENDERING_BIT:
				return "CONDITIONAL_RENDERING_BIT";
			case PipelineStageBit::FRAGMENT_SHADING_RATE_ATTACHMENT_BIT:
				return "FRAGMENT_SHADING_RATE_ATTACHMENT_BIT";
			case PipelineStageBit::ACCELERATION_STRUCTURE_BUILD_BIT:
				return "ACCELERATION_STRUCTURE_BUILD_BIT";
			case PipelineStageBit::RAY_TRACING_SHADER_BIT:
				return "RAY_TRACING_SHADER_BIT";
			case PipelineStageBit::FRAGMENT_DENSITY_PROCESS_BIT:
				return "FRAGMENT_DENSITY_PROCESS_BIT";
			case PipelineStageBit::TASK_SHADER_BIT_EXT:
				return "TASK_SHADER_BIT_EXT";
			case PipelineStageBit::MESH_SHADER_BIT_EXT:
				return "MESH_SHADER_BIT_EXT";
			case PipelineStageBit::VIDEO_DECODE_BIT:
				return "VIDEO_DECODE_BIT";
			case PipelineStageBit::VIDEO_ENCODE_BIT:
				return "VIDEO_ENCODE_BIT";
			case PipelineStageBit::COPY_BIT:
				return "COPY_BIT";
			case PipelineStageBit::RESOLVE_BIT:
				return "RESOLVE_BIT";
			case PipelineStageBit::BLIT_BIT:
				return "BLIT_BIT";
			case PipelineStageBit::CLEAR_BIT:
				return "CLEAR_BIT";
			case PipelineStageBit::INDEX_INPUT_BIT:
				return "INDEX_INPUT_BIT";
			case PipelineStageBit::VERTEX_ATTRIBUTE_INPUT_BIT:
				return "VERTEX_ATTRIBUTE_INPUT_BIT";
			case PipelineStageBit::PRE_RASTERIZATION_SHADERS_BIT:
				return "PRE_RASTERIZATION_SHADERS_BIT";
			default:
				return "";
			}
		};

	if (pipelingStages == PipelineStageBit::NONE_BIT)
	{
		return GetSinglePipelineStageString(pipelingStages);
	}
	else
	{
		static_assert(sizeof(PipelineStageBit) == sizeof(uint64_t));
		std::string result = "";
		bool first = true;
		for (uint64_t i = 1; !(i > static_cast<uint64_t>(PipelineStageBit::PRE_RASTERIZATION_SHADERS_BIT)); i *= 2)
		{
			PipelineStageBit stage = static_cast<PipelineStageBit>(i);
			if (arc::BitwiseCheck(pipelingStages, stage))
			{
				result += (!first ? " | " : "") + GetSinglePipelineStageString(stage);
				first = false;
			}
		}
		return result;
	}
}
