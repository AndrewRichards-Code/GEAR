#include "gear_core_common.h"
#include "renderpipeline.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

RenderPipeline::RenderPipeline(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	for (auto& shaderCI : m_CI.shaderCreateInfo)
	{
		m_Shaders.emplace_back(Shader::Create(&shaderCI));
	}
	m_PipelineCI.shaders = m_Shaders;
	m_Device = m_CI.shaderCreateInfo[0].device;
	
	FinalisePipline();
}

RenderPipeline::~RenderPipeline()
{
}

void RenderPipeline::FinalisePipline()
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
					m_DescSetLayoutCIs[set].debugName = "GEAR_CORE_DescSetLayout_Set";
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
		m_PipelineCI.viewportState = m_CI.viewportState;
		m_PipelineCI.rasterisationState = m_CI.rasterisationState;
		m_PipelineCI.multisampleState = m_CI.multisampleState;
		m_PipelineCI.depthStencilState = m_CI.depthStencilState;
		m_PipelineCI.colourBlendState = m_CI.colourBlendState;
		m_PipelineCI.dynamicStates = {};
		m_PipelineCI.layout.descriptorSetLayouts = m_DescSetLayouts;
		m_PipelineCI.layout.pushConstantRanges = {};
		m_PipelineCI.renderPass = m_CI.renderPass;
		m_PipelineCI.subpassIndex = m_CI.subpassIndex;
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
				m_DescSetLayoutCIs[set].debugName = "GEAR_CORE_DescSetLayout_Set";
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

void RenderPipeline::RecompileShaders()
{
	for (auto& shader : m_Shaders)
		shader->Recompile();

	m_PipelineCI.shaders = m_Shaders;
	Rebuild();
}

void RenderPipeline::Rebuild()
{
	m_PipelineCI.viewportState = m_CI.viewportState;
	m_PipelineCI.rasterisationState = m_CI.rasterisationState;
	m_PipelineCI.multisampleState = m_CI.multisampleState;
	m_PipelineCI.depthStencilState = m_CI.depthStencilState;
	m_PipelineCI.colourBlendState = m_CI.colourBlendState;
	m_PipelineCI.renderPass = m_CI.renderPass;
	m_PipelineCI.subpassIndex = m_CI.subpassIndex;
	m_Pipeline = crossplatform::Pipeline::Create(&m_PipelineCI);
}
