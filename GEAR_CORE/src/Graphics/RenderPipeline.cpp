#include "gear_core_common.h"
#include "RenderPipeline.h"
#include "Utils/ModelLoader.h"

#include "Core/EnumStringMaps.h"
#include "Core/JsonFileHelper.h"

using namespace gear;
using namespace core;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

RenderPipeline::ShaderBuildMode RenderPipeline::s_ShaderBuildMode = RenderPipeline::ShaderBuildMode::INCREMENTAL;

RenderPipeline::RenderPipeline(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	for (auto& shaderCI : m_CI.shaderCreateInfo)
	{
		m_Shaders.emplace_back(Shader::Create(&shaderCI));

		if (s_ShaderBuildMode == ShaderBuildMode::NEVER)
		{
			continue;
		}
		else if (s_ShaderBuildMode == ShaderBuildMode::INCREMENTAL)
		{
			if (std::filesystem::last_write_time(shaderCI.binaryFilepath) < std::filesystem::last_write_time(shaderCI.recompileArguments.hlslFilepath))
			{
				m_Shaders.back()->Recompile();
			}
		}
		else if (s_ShaderBuildMode == ShaderBuildMode::ALWAYS)
		{
			m_Shaders.back()->Recompile();
		}
		else
		{
			continue;
		}
	}
	m_PipelineCI.shaders = m_Shaders;
	m_Device = m_CI.shaderCreateInfo[0].device;
	
	FinalisePipline();
}

RenderPipeline::RenderPipeline(LoadInfo* pLoadInfo)
{
	using namespace nlohmann;

	std::string projectDirectory = PROJECT_DIR;
	std::string finalFilePath = projectDirectory + pLoadInfo->filepath;
	std::string relativePathFromCwdToProjDir = std::filesystem::path(projectDirectory).lexically_relative(std::filesystem::current_path()).string();
	
	json pipeline_grpf;
	core::LoadJsonFile(finalFilePath, ".grpf", "GEAR_RENDER_PIPELINE_FILE", pipeline_grpf);
	
	RenderPipeline::CreateInfo rpCI = {};
	rpCI.debugName = pipeline_grpf["debugName"];

	//Shaders
	for (auto& shader : pipeline_grpf["shaders"])
	{
		Shader::CreateInfo shaderCI;
		shaderCI.debugName = shader["debugName"];
		shaderCI.device = pLoadInfo->device;
		for (auto& stageAndEntryPoint : shader["stageAndEntryPoints"])
		{
			shaderCI.stageAndEntryPoints.push_back({ShaderStageBitStrings[stageAndEntryPoint["stage"]], stageAndEntryPoint["entryPoint"]});
		}
		shaderCI.binaryFilepath = projectDirectory + std::string(shader["binaryFilepath"]);
		shaderCI.binaryCode = {};

		json& recompileArgs = shader["recompileArguments"];
		shaderCI.recompileArguments.hlslFilepath = relativePathFromCwdToProjDir + std::string(recompileArgs["hlslFilepath"]);
		shaderCI.recompileArguments.outputDirectory = relativePathFromCwdToProjDir + std::string(recompileArgs["outputDirectory"]);
		for (auto& includeDir : recompileArgs["includeDirectories"])
			shaderCI.recompileArguments.includeDirectories.push_back(relativePathFromCwdToProjDir + std::string(includeDir));
		shaderCI.recompileArguments.entryPoint = recompileArgs["entryPoint"];
		shaderCI.recompileArguments.shaderModel = recompileArgs["shaderModel"];
		for (auto& macro : recompileArgs["macros"])
			shaderCI.recompileArguments.macros.push_back(macro);
		shaderCI.recompileArguments.cso = recompileArgs["cso"];
		shaderCI.recompileArguments.spv = recompileArgs["spv"];
		shaderCI.recompileArguments.dxcLocation = recompileArgs["dxcLocation"];
		for (auto& dxcArguments : recompileArgs["dxcArguments"])
			shaderCI.recompileArguments.dxcArguments.push_back(dxcArguments);

		rpCI.shaderCreateInfo.push_back(shaderCI);
	}

	//Compute Pipelines are completed here.
	if (rpCI.shaderCreateInfo.size() == 1 
		&& rpCI.shaderCreateInfo.back().stageAndEntryPoints.size() == 1 
		&& rpCI.shaderCreateInfo.back().stageAndEntryPoints.back().first == Shader::StageBit::COMPUTE_BIT)
	{
		*this = graphics::RenderPipeline(&rpCI);
		return;
	}

	//InputAssembly
	json& inputAssemblyState = pipeline_grpf["inputAssemblyState"];
	rpCI.inputAssemblyState.topology = PrimitiveTopologyStrings[inputAssemblyState["topology"]];
	rpCI.inputAssemblyState.primitiveRestartEnable = inputAssemblyState["primitiveRestartEnable"];

	//ViewportState
	json& viewportState = pipeline_grpf["viewportState"];
	for (auto& viewport : viewportState["viewports"])
	{
		rpCI.viewportState.viewports.push_back({
			viewport["x"],
			viewport["y"],
			(viewport["width"].is_string() && std::string(viewport["width"]).compare("VIEWPORT_WIDTH") == 0) ? pLoadInfo->viewportWidth : viewport["width"],
			(viewport["height"].is_string() && std::string(viewport["height"]).compare("VIEWPORT_HEIGHT") == 0) ? pLoadInfo->viewportHeight : viewport["height"],
			viewport["minDepth"],
			viewport["maxDepth"]
			});
	}
	for (auto& scissor : viewportState["scissors"])
	{
		rpCI.viewportState.scissors.push_back({
			{
				scissor["x"],
				scissor["y"]
			},
			{
				(uint32_t)((scissor["width"].is_string() && std::string(scissor["width"]).compare("VIEWPORT_WIDTH") == 0) ? pLoadInfo->viewportWidth : scissor["width"]),
				(uint32_t)((scissor["height"].is_string() && std::string(scissor["height"]).compare("VIEWPORT_HEIGHT") == 0) ? pLoadInfo->viewportHeight : scissor["height"]),
			}
			});
	}

	//RasterisationState
	json& rasterisationState = pipeline_grpf["rasterisationState"];
	rpCI.rasterisationState.depthClampEnable = rasterisationState["depthClampEnable"];
	rpCI.rasterisationState.rasteriserDiscardEnable = rasterisationState["depthClampEnable"];
	rpCI.rasterisationState.polygonMode = PolygonModeStrings[rasterisationState["polygonMode"]];
	rpCI.rasterisationState.cullMode = CullModeBitStrings[rasterisationState["cullMode"]];
	rpCI.rasterisationState.frontFace = FrontFaceStrings[rasterisationState["frontFace"]];
	rpCI.rasterisationState.depthBiasEnable = rasterisationState["depthBiasEnable"];
	rpCI.rasterisationState.depthBiasConstantFactor = rasterisationState["depthBiasConstantFactor"];
	rpCI.rasterisationState.depthBiasClamp = rasterisationState["depthBiasClamp"];
	rpCI.rasterisationState.depthBiasSlopeFactor = rasterisationState["depthBiasSlopeFactor"];
	rpCI.rasterisationState.lineWidth = rasterisationState["lineWidth"];

	//MultisampleState
	json& multisampleState = pipeline_grpf["multisampleState"];
	rpCI.multisampleState.rasterisationSamples = std::string(multisampleState["rasterisationSamples"]).compare("FRAMEBUFFER_SAMPLE_COUNT") == 0 ? pLoadInfo->samples : SampleCountBitStrings[multisampleState["rasterisationSamples"]];
	rpCI.multisampleState.sampleShadingEnable = multisampleState["sampleShadingEnable"];
	rpCI.multisampleState.minSampleShading = multisampleState["minSampleShading"];
	rpCI.multisampleState.sampleMask = multisampleState["sampleMask"];
	rpCI.multisampleState.alphaToCoverageEnable = multisampleState["alphaToCoverageEnable"];
	rpCI.multisampleState.alphaToOneEnable = multisampleState["alphaToOneEnable"];

	//DepthStencilState
	json& depthStencilState = pipeline_grpf["depthStencilState"];
	rpCI.depthStencilState.depthTestEnable = depthStencilState["depthTestEnable"];
	rpCI.depthStencilState.depthWriteEnable = depthStencilState["depthWriteEnable"];
	rpCI.depthStencilState.depthCompareOp = CompareOpStrings[depthStencilState["depthCompareOp"]];
	rpCI.depthStencilState.depthBoundsTestEnable = depthStencilState["depthBoundsTestEnable"];
	rpCI.depthStencilState.stencilTestEnable = depthStencilState["stencilTestEnable"];
	if (rpCI.depthStencilState.stencilTestEnable)
	{
		json& front = depthStencilState["front"];
		rpCI.depthStencilState.front.failOp = StencilOpStrings[front["failOp"]];
		rpCI.depthStencilState.front.passOp = StencilOpStrings[front["passOp"]];
		rpCI.depthStencilState.front.depthFailOp = StencilOpStrings[front["depthFailOp"]];
		rpCI.depthStencilState.front.compareOp = CompareOpStrings[front["compareOp"]];
		rpCI.depthStencilState.front.compareMask = front["compareMask"];
		rpCI.depthStencilState.front.writeMask = front["writeMask"];
		rpCI.depthStencilState.front.reference = front["reference"];
		json& back = depthStencilState["back"];
		rpCI.depthStencilState.back.failOp = StencilOpStrings[back["failOp"]];
		rpCI.depthStencilState.back.passOp = StencilOpStrings[back["passOp"]];
		rpCI.depthStencilState.back.depthFailOp = StencilOpStrings[back["depthFailOp"]];
		rpCI.depthStencilState.back.compareOp = CompareOpStrings[back["compareOp"]];
		rpCI.depthStencilState.back.compareMask = back["compareMask"];
		rpCI.depthStencilState.back.writeMask = back["writeMask"];
		rpCI.depthStencilState.back.reference = back["reference"];
	}
	else
	{
		rpCI.depthStencilState.front = {};
		rpCI.depthStencilState.back = {};
	}
	rpCI.depthStencilState.minDepthBounds = depthStencilState["minDepthBounds"];
	rpCI.depthStencilState.maxDepthBounds = depthStencilState["maxDepthBounds"];

	//ColourBlendState
	json& colourBlendState = pipeline_grpf["colourBlendState"];
	rpCI.colourBlendState.logicOpEnable = colourBlendState["logicOpEnable"];
	rpCI.colourBlendState.logicOp = LogicOpStrings[colourBlendState["logicOp"]];
	for (auto& attachment : colourBlendState["attachments"])
	{
		ColourComponentBit ccb = ColourComponentBit(0);
		for (auto& cc : attachment["colourWriteMask"])
			ccb |= ColourComponentBitStrings[cc];

		rpCI.colourBlendState.attachments.push_back({
			attachment["blendEnable"],
			BlendFactorStrings[attachment["srcColourBlendFactor"]],
			BlendFactorStrings[attachment["dstColourBlendFactor"]],
			BlendOpStrings[attachment["colourBlendOp"]],
			BlendFactorStrings[attachment["srcAlphaBlendFactor"]],
			BlendFactorStrings[attachment["dstAlphaBlendFactor"]],
			BlendOpStrings[attachment["alphaBlendOp"]],
			ccb
			});
	}
	rpCI.colourBlendState.blendConstants[0] = colourBlendState["blendConstants"][0];
	rpCI.colourBlendState.blendConstants[1] = colourBlendState["blendConstants"][1];
	rpCI.colourBlendState.blendConstants[2] = colourBlendState["blendConstants"][2];
	rpCI.colourBlendState.blendConstants[3] = colourBlendState["blendConstants"][3];

	//RenderPass
	rpCI.renderPass = pLoadInfo->renderPass;
	rpCI.subpassIndex = pLoadInfo->subpassIndex;

	*this = graphics::RenderPipeline(&rpCI);
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

			viads.push_back({ vsiad.location, vsiad.binding, vsiad.vertexType, vsiad.offset, vsiad.semanticName.c_str() });
		}
		std::vector<VertexInputBindingDescription> vibds;
		if (stride > 0)
		{
			vibds.push_back({ 0, stride, VertexInputRate::VERTEX });
		}

		for (auto& shader : m_Shaders)
		{
			for (auto& rbds : shader->GetRBDs())
			{
				uint32_t set = rbds.first;
				m_DescSetLayoutCIs.resize(std::max((size_t)set + 1, m_DescSetLayoutCIs.size()));
				m_RBDs.resize(std::max((size_t)set + 1, m_RBDs.size()));

				for (auto& rbd : rbds.second)
				{
					m_DescSetLayoutCIs[set].debugName = "GEAR_CORE_Pipeline_DescSetLayout_Set_" + std::to_string(set) + ": " + m_CI.debugName;;
					m_DescSetLayoutCIs[set].device = m_Device;
					m_DescSetLayoutCIs[set].descriptorSetLayoutBinding.push_back({
						rbd.second.binding,
						rbd.second.type,
						rbd.second.descriptorCount,
						rbd.second.stage });

					m_RBDs[set].push_back(rbd.second);
				}
			}
		}
		for (auto& descSetLayoutCI : m_DescSetLayoutCIs)
		{
			m_DescSetLayouts.emplace_back(DescriptorSetLayout::Create(&descSetLayoutCI));
		}

		m_PipelineCI.debugName = "GEAR_CORE_Pipeline: " + m_CI.debugName;
		m_PipelineCI.device = m_Device;
		m_PipelineCI.type = PipelineType::GRAPHICS;
		m_PipelineCI.vertexInputState.vertexInputBindingDescriptions = vibds;
		m_PipelineCI.vertexInputState.vertexInputAttributeDescriptions = viads;
		m_PipelineCI.inputAssemblyState = m_CI.inputAssemblyState;
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
			m_DescSetLayoutCIs.resize(std::max((size_t)set + 1, m_DescSetLayoutCIs.size()));
			m_RBDs.resize(std::max((size_t)set + 1, m_RBDs.size()));

			for (auto& rbd : rbds.second)
			{
				m_DescSetLayoutCIs[set].debugName = "GEAR_CORE_Pipeline_DescSetLayout_Set_" + std::to_string(set) + ": " + m_CI.debugName;
				m_DescSetLayoutCIs[set].device = m_Device;
				m_DescSetLayoutCIs[set].descriptorSetLayoutBinding.push_back({
					rbd.second.binding,
					rbd.second.type,
					rbd.second.descriptorCount,
					rbd.second.stage });

				m_RBDs[set].push_back(rbd.second);
			}
		}
		for (auto& descSetLayoutCI : m_DescSetLayoutCIs)
		{
			m_DescSetLayouts.emplace_back(DescriptorSetLayout::Create(&descSetLayoutCI));
		}

		m_PipelineCI.debugName = "GEAR_CORE_Pipeline: " + m_CI.debugName;
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
	{
		if (s_ShaderBuildMode == ShaderBuildMode::NEVER)
		{
			continue;
		}
		else if (s_ShaderBuildMode == ShaderBuildMode::INCREMENTAL)
		{
			if (std::filesystem::last_write_time(shader->GetCreateInfo().binaryFilepath) < std::filesystem::last_write_time(shader->GetCreateInfo().recompileArguments.hlslFilepath))
			{
				shader->Recompile();
			}
		}
		else if (s_ShaderBuildMode == ShaderBuildMode::ALWAYS)
		{
			shader->Recompile();
		}
		else
		{
			continue;
		}
	}

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
