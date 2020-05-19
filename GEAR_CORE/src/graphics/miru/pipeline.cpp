#include "pipeline.h"

using namespace miru;
using namespace miru::crossplatform;

GEAR::GRAPHICS::Pipeline::Pipeline(void* device, const std::string& vertexPath, const std::string& fragmentPath)
	:m_Device(device)
{
	AddAdditionalShaderModule(Shader::StageBit::VERTEX_BIT, vertexPath);
	AddAdditionalShaderModule(Shader::StageBit::FRAGMENT_BIT, fragmentPath);
}

GEAR::GRAPHICS::Pipeline::Pipeline(void* device, const std::string& computePath)
{
	AddAdditionalShaderModule(Shader::StageBit::COMPUTE_BIT, computePath);
}

GEAR::GRAPHICS::Pipeline::~Pipeline()
{
}

void GEAR::GRAPHICS::Pipeline::AddAdditionalShaderModule(Shader::StageBit stage, const std::string& shaderPath)
{
	Shader::CreateInfo shaderCI;
	shaderCI.debugName = shaderPath.substr(shaderPath.find_last_of('/')).c_str();
	shaderCI.device = m_Device;
	shaderCI.stage = stage;
	shaderCI.entryPoint = "main";
	shaderCI.binaryFilepath = shaderPath.c_str();
	shaderCI.binaryCode = {};
	shaderCI.recompileArguments = {};
	m_Shaders.emplace_back(Shader::Create(&shaderCI));
	m_PipelineCI.shaders = m_Shaders;
}

void GEAR::GRAPHICS::Pipeline::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	m_ViewportState.viewports.push_back({ x, y, width, height, minDepth, maxDepth });
	m_ViewportState.scissors.push_back({{(int32_t)x, (int32_t)y}, {(uint32_t)width, (uint32_t)height}});
}

void GEAR::GRAPHICS::Pipeline::SetRasterisationState(bool depthClampEnable, bool rasteriserDiscardEnable, PolygonMode polygonMode, CullModeBit cullMode,
	FrontFace frontFace, bool depthBiasEnable, float depthBiasConstantFactor, float depthBiasClamp,  float depthBiasSlopeFactor, float lineWidth)
{
	m_RasterisationState.depthClampEnable = depthClampEnable;
	m_RasterisationState.rasteriserDiscardEnable = rasteriserDiscardEnable;
	m_RasterisationState.polygonMode = polygonMode;
	m_RasterisationState.cullMode = cullMode;
	m_RasterisationState.frontFace = frontFace;
	m_RasterisationState.depthBiasEnable = depthBiasEnable;
	m_RasterisationState.depthBiasConstantFactor = depthBiasConstantFactor;
	m_RasterisationState.depthBiasClamp = depthBiasClamp;
	m_RasterisationState.depthBiasSlopeFactor = depthBiasSlopeFactor;
	m_RasterisationState.lineWidth = lineWidth;
}

void GEAR::GRAPHICS::Pipeline::SetMultisampleState(Image::SampleCountBit rasterisationSamples, bool sampleShadingEnable, float minSampleShading,
	bool alphaToCoverageEnable, bool alphaToOneEnable)
{
	m_MultisampleState.rasterisationSamples = rasterisationSamples;
	m_MultisampleState.sampleShadingEnable = sampleShadingEnable;
	m_MultisampleState.minSampleShading = minSampleShading;
	m_MultisampleState.alphaToCoverageEnable = alphaToCoverageEnable;
	m_MultisampleState.alphaToOneEnable = alphaToOneEnable;
}

void GEAR::GRAPHICS::Pipeline::SetDepthStencilState(bool depthTestEnable, bool depthWriteEnable, CompareOp depthCompareOp, bool depthBoundsTestEnable,
	bool stencilTestEnable, const StencilOpState& front, const StencilOpState& back, float minDepthBounds, float maxDepthBounds)
{
	m_DepthStencilState.depthTestEnable = depthTestEnable;
	m_DepthStencilState.depthWriteEnable = depthWriteEnable;
	m_DepthStencilState.depthCompareOp = depthCompareOp;
	m_DepthStencilState.depthBoundsTestEnable = depthBoundsTestEnable;
	m_DepthStencilState.stencilTestEnable = stencilTestEnable;
	m_DepthStencilState.front = front;
	m_DepthStencilState.back = back;
	m_DepthStencilState.minDepthBounds = minDepthBounds;
	m_DepthStencilState.maxDepthBounds = maxDepthBounds;
}

void GEAR::GRAPHICS::Pipeline::SetColourBlendState(bool logicOpEnable, LogicOp logicOp, const std::vector<ColourBlendAttachmentState> attachments, float blendConstant[4])
{
	m_ColourBlendState.logicOpEnable = logicOpEnable;
	m_ColourBlendState.logicOp = logicOp;
	m_ColourBlendState.attachments = attachments;
	m_ColourBlendState.blendConstants[0] = blendConstant[0];
	m_ColourBlendState.blendConstants[1] = blendConstant[1];
	m_ColourBlendState.blendConstants[2] = blendConstant[2];
	m_ColourBlendState.blendConstants[3] = blendConstant[3];
}

void GEAR::GRAPHICS::Pipeline::FinalisePipline()
{
	if (m_Shaders.size() > 1)
	{
		std::vector<VertexInputBindingDescription> vibds;
		std::vector<VertexInputAttributeDescription> viads;
		uint32_t i = 0;
		for (auto& vsiad : m_Shaders[0]->GetVSIADs())
		{
			uint32_t typeSize = 4;
			if (vsiad.vertexType > VertexType::VEC4 && vsiad.vertexType < VertexType::INT)
				typeSize = 8;

			vibds.push_back({ i, (((uint32_t)vsiad.vertexType % 4) + 1) * typeSize, VertexInputRate::VERTEX });
			viads.push_back({ vsiad.location, vsiad.binding + i, vsiad.vertexType, 0, vsiad.semanticName.c_str() });
			i++;
		}

		for (auto& shader : m_Shaders)
		{
			for (auto& rbds : shader->GetRBDs())
			{
				uint32_t set = rbds.first;
				m_DescSetLayoutCIs.resize(std::max((size_t)set + 1, m_DescSetLayoutCIs.size()));

				for (auto& rbd : rbds.second)
				{
					m_DescSetLayoutCIs[set].debugName = (std::string("GEAR_CORE_DescSetLayout_Set:") + std::to_string(set)).c_str();
					m_DescSetLayoutCIs[set].device = m_Device;
					m_DescSetLayoutCIs[set].descriptorSetLayoutBinding.push_back({
						rbd.binding,
						rbd.type,
						rbd.descriptorCount,
						rbd.stage });
				}
			}
		}
		for (auto& descSetLayoutCI : m_DescSetLayoutCIs)
		{
			m_DescSetLayouts.emplace_back(DescriptorSetLayout::Create(&descSetLayoutCI));
		}

		m_PipelineCI.debugName = "GEAR_CORE_Pipeline";
		m_PipelineCI.device = m_Device;
		m_PipelineCI.type = PipelineType::GRAPHICS;
		m_PipelineCI.vertexInputState.vertexInputBindingDescriptions = vibds;
		m_PipelineCI.vertexInputState.vertexInputAttributeDescriptions = viads;
		m_PipelineCI.inputAssemblyState = { PrimitiveTopology::TRIANGLE_LIST, false };
		m_PipelineCI.tessellationState = {};
		m_PipelineCI.viewportState = m_ViewportState;
		m_PipelineCI.rasterisationState = m_RasterisationState;
		m_PipelineCI.multisampleState = m_MultisampleState;
		m_PipelineCI.depthStencilState = m_DepthStencilState;
		m_PipelineCI.colourBlendState = m_ColourBlendState;
		m_PipelineCI.dynamicStates = {};
		m_PipelineCI.layout.descriptorSetLayouts = m_DescSetLayouts;
		m_PipelineCI.layout.pushConstantRanges = {};
		m_PipelineCI.renderPass = m_RenderPass;
		m_PipelineCI.subpassIndex = m_SubpassIndex;
		m_Pipeline = crossplatform::Pipeline::Create(&m_PipelineCI);
	}
	else
	{
		for (auto& rbds : m_Shaders[0]->GetRBDs())
		{
			uint32_t set = rbds.first;
			m_DescSetLayoutCIs.resize(std::max((size_t)set, m_DescSetLayoutCIs.size()));

			for (auto& rbd : rbds.second)
			{
				m_DescSetLayoutCIs[set].debugName = (std::string("GEAR_CORE_DescSetLayout_Set:") + std::to_string(set)).c_str();
				m_DescSetLayoutCIs[set].device = m_Device;
				m_DescSetLayoutCIs[set].descriptorSetLayoutBinding.push_back({
					rbd.binding,
					rbd.type,
					rbd.descriptorCount,
					rbd.stage });
			}
		}
		for (auto& descSetLayoutCI : m_DescSetLayoutCIs)
		{
			m_DescSetLayouts.emplace_back(DescriptorSetLayout::Create(&descSetLayoutCI));
		}

		m_PipelineCI.debugName = "GEAR_CORE_Pipeline";
		m_PipelineCI.device = m_Device;
		m_PipelineCI.type = PipelineType::COMPUTE;
		m_PipelineCI.shaders = m_Shaders;
		m_PipelineCI.layout.descriptorSetLayouts = m_DescSetLayouts;
		m_PipelineCI.layout.pushConstantRanges = {};
		m_Pipeline = crossplatform::Pipeline::Create(&m_PipelineCI);
	}
}