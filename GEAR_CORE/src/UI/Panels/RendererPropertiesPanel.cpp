#include "gear_core_common.h"
#include "UI/Panels/RendererPropertiesPanel.h"
#include "UI/UIContext.h"
#include "UI/ComponentUI/ComponentUIs.h"

#include "Graphics/Rendering/Renderer.h"
#include "Graphics/Rendering/RenderGraph.h"
#include "Core/ParseStack.h"

#include "ImGuizmo/GraphEditor.h"

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
	DrawRenderGraph();

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

class RenderGraphEditorDelegate final : public GraphEditor::Delegate
{
public:
	bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override
	{
		return false;
	}

	void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) override
	{
		m_Nodes[nodeIndex].selected = selected;
	}

	void MoveSelectedNodes(const ImVec2 delta) override
	{
		for (auto& node : m_Nodes)
		{
			if (!node.selected)
			{
				continue;
			}
			node.x += delta.x;
			node.y += delta.y;
		}
	}

	virtual void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) override
	{
	}

	void AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) override
	{
		for (const auto& link : m_Links)
		{
			if (link.mInputNodeIndex == inputNodeIndex && link.mInputSlotIndex == inputSlotIndex
				&& link.mOutputNodeIndex == outputNodeIndex && link.mOutputSlotIndex == outputSlotIndex)
				return;

		}
		m_Links.push_back({ inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex });
	}

	void DelLink(GraphEditor::LinkIndex linkIndex) override
	{
		m_Links.erase(m_Links.begin() + linkIndex);
	}

	void CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) override
	{
		drawList->AddLine(rectangle.Min, rectangle.Max, IM_COL32(0, 0, 0, 255));
		drawList->AddText(rectangle.Min, IM_COL32(255, 128, 64, 255), "Draw");
	}

	const size_t GetTemplateCount() override
	{
		return m_Templates.size();
	}

	const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override
	{
		return m_Templates[index];
	}

	const size_t GetNodeCount() override
	{
		return m_Nodes.size();
	}

	const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override
	{
		const auto& myNode = m_Nodes[index];
		GraphEditor::Node result;
		result.mName = myNode.name.c_str();
		result.mTemplateIndex = myNode.templateIndex;
		result.mRect.Min.x = myNode.x;
		result.mRect.Min.y = myNode.y;
		result.mRect.Max.x = myNode.x + 200;
		result.mRect.Max.y = myNode.y + 200;
		result.mSelected = myNode.selected;
		return result;
	}

	const size_t GetLinkCount() override
	{
		return m_Links.size();
	}

	const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override
	{
		return m_Links[index];
	}

	size_t GetTemplate(uint8_t inputCount, uint8_t outputCount)
	{
		for (size_t i = 0; i < m_Templates.size(); i++)
		{
			const GraphEditor::Template _template = m_Templates[i];
			if (_template.mInputCount == inputCount && _template.mOutputCount == outputCount)
			{
				return i;
			}
		}

		m_Templates.push_back({
			IM_COL32(160, 160, 180, 255),
			IM_COL32(100, 100, 140, 255),
			IM_COL32(110, 110, 150, 255),
			inputCount,
			nullptr,
			nullptr,
			outputCount,
			nullptr,
			nullptr
		});
		return m_Templates.size() - 1;
	}

	void Reset()
	{
		m_Nodes.clear();
		m_Links.clear();
		m_Templates.clear();
	}

	void Initialise(Ref<Renderer>& renderer, const RenderGraph& renderGraph)
	{
		const uint32_t& frameIndex = renderer->GetFrameIndex();
		const auto& passes = renderGraph.GetPasses(frameIndex);
		const auto& adjacencyLists = renderGraph.GetAdjacencyLists(frameIndex);

		if (passes.size() == m_Nodes.size())
		{
			return;
		}

		Reset();

		float x = 0.0f;
		float y = 0.0f;
		size_t i = 0;
		for (const auto& pass : passes)
		{
			const uint8_t inputCount = static_cast<uint8_t>(pass->GetInputResourceViews().size());
			const uint8_t outputCount = static_cast<uint8_t>(pass->GetOutputResourceViews().size());
			GraphEditor::TemplateIndex templateIndex = GetTemplate(inputCount, outputCount);

			m_Nodes.push_back({
				pass->GetName(),
				templateIndex,
				x, y,
				false
				});
			x += 400.0f;

			for (const size_t& otherPassIndex : adjacencyLists[i])
			{
				AddLink(i, 0, otherPassIndex, 0);
			}

			i++;
		}

	}

private:
	struct Node
	{
		std::string name;
		GraphEditor::TemplateIndex templateIndex;
		float x, y;
		bool selected;
	};

	std::vector<Node> m_Nodes;
	std::vector<GraphEditor::Link> m_Links;
	std::vector<GraphEditor::Template> m_Templates;
};

static RenderGraphEditorDelegate rged;
static GraphEditor::Options options;
static GraphEditor::ViewState viewState;
static GraphEditor::FitOnScreen fit = GraphEditor::Fit_None;

void RendererPropertiesPanel::DrawRenderGraph()
{
	if (m_ViewportPanel)
	{
		Ref<Renderer>& renderer = m_ViewportPanel->GetCreateInfo().renderer;
		const RenderGraph& renderGraph = renderer->GetRenderGraph();

		rged.Initialise(renderer, renderGraph);
	}

	ImGui::Begin("Graph Editor", nullptr, 0);
	GraphEditor::Show(rged, options, viewState, true, &fit);
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
