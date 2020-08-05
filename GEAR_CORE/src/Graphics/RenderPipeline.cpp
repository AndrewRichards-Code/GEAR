#include "gear_core_common.h"
#include "RenderPipeline.h"
#include "Utils/ModelLoader.h"

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
		uint32_t stride = 0;
		std::vector<VertexInputAttributeDescription> viads;
		for (auto& vsiad : m_Shaders[0]->GetVSIADs())
		{
			uint32_t typeSize = (vsiad.vertexType > VertexType::VEC4 && vsiad.vertexType < VertexType::INT) ? 8 : 4;
			stride += (((uint32_t)vsiad.vertexType % 4) + 1) * typeSize;

			viads.push_back({ vsiad.location, vsiad.binding, vsiad.vertexType, 0, vsiad.semanticName.c_str() });
		}
		std::vector<VertexInputBindingDescription> vibds;
		vibds.push_back({ 0, stride, VertexInputRate::VERTEX });

		for (auto& shader : m_Shaders)
		{
			for (auto& rbds : shader->GetRBDs())
			{
				uint32_t set = rbds.first;
				m_DescSetLayoutCIs.resize(std::max((size_t)set + 1, m_DescSetLayoutCIs.size()));
				m_DescSetLayoutDebugNames.resize(m_DescSetLayoutCIs.size());

				for (auto& rbd : rbds.second)
				{
					m_DescSetLayoutDebugNames[set] = std::string("GEAR_CORE_Pipeline_DescSetLayout_Set_") 
						+ std::to_string(set) + std::string(": ") + std::string(m_CI.debugName);
					
					m_DescSetLayoutCIs[set].debugName = m_DescSetLayoutDebugNames[set].c_str();
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

		m_PipelineDebugName = std::string("GEAR_CORE_Pipeline: ") + m_CI.debugName;
		m_PipelineCI.debugName = m_PipelineDebugName.c_str();
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
			m_DescSetLayoutDebugNames.resize(m_DescSetLayoutCIs.size());

			for (auto& rbd : rbds.second)
			{
				m_DescSetLayoutDebugNames[set] = std::string("GEAR_CORE_Pipeline_DescSetLayout_Set_")
					+ std::to_string(set) + std::string(": ") + std::string(m_CI.debugName);

				m_DescSetLayoutCIs[set].debugName = m_DescSetLayoutDebugNames[set].c_str();
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

		m_PipelineDebugName = std::string("GEAR_CORE_Pipeline: ") + m_CI.debugName;
		m_PipelineCI.debugName = m_PipelineDebugName.c_str();
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
