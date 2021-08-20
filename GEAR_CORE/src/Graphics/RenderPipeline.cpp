#include "gear_core_common.h"
#include "RenderPipeline.h"
#include "ARC/src/FileSystemHelpers.h"
#include "Utils/ModelLoader.h"

#include "Core/EnumStringMaps.h"

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
			if (arc::FileLastWriteTime(shaderCI.binaryFilepath) < arc::FileLastWriteTime(shaderCI.recompileArguments.hlslFilepath))
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
	
	json pipeline_grpf_json;
	std::ifstream pipeline_grpf(finalFilePath, std::ios::binary);
	if (pipeline_grpf.is_open())
	{
		pipeline_grpf >> pipeline_grpf_json;
	}
	else
	{
		GEAR_WARN(ErrorCode::GRAPHICS | ErrorCode::NO_FILE, "Unable to open %s.", finalFilePath.c_str());
		return;
	}

	if (pipeline_grpf_json.empty())
	{
		GEAR_WARN(ErrorCode::GRAPHICS | ErrorCode::LOAD_FAILED, "%s is not valid.", finalFilePath.c_str());
		return;
	}

	std::string fileType = pipeline_grpf_json["fileType"];
	if (fileType.compare("GEAR_RENDER_PIPELINE_FILE") != 0)
	{
		GEAR_WARN(ErrorCode::GRAPHICS | ErrorCode::NOT_SUPPORTED, "%s is not valid.", finalFilePath.c_str());
		return;
	}

	RenderPipeline::CreateInfo rpCI = {};
	rpCI.debugName = pipeline_grpf_json["debugName"];

	//Shaders
	for (auto& shader : pipeline_grpf_json["shaders"])
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

		auto& recompileArgs = shader["recompileArguments"];
		shaderCI.recompileArguments.mscDirectory = relativePathFromCwdToProjDir + std::string(recompileArgs["mscDirectory"]);
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
		shaderCI.recompileArguments.dxcArguments = recompileArgs["dxcArguments"];
		shaderCI.recompileArguments.nologo = recompileArgs["nologo"];
		shaderCI.recompileArguments.nooutput = recompileArgs["nooutput"];

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
	rpCI.inputAssemblyState.topology = PrimitiveTopologyStrings[pipeline_grpf_json["inputAssemblyState"]["topology"]];
	rpCI.inputAssemblyState.primitiveRestartEnable = pipeline_grpf_json["inputAssemblyState"]["primitiveRestartEnable"];

	//ViewportState
	for (auto& viewport : pipeline_grpf_json["viewportState"]["viewports"])
	{
		rpCI.viewportState.viewports.push_back({
			viewport["x"],
			viewport["y"],
			std::string(viewport["width"]).compare("VIEWPORT_WIDTH") == 0 ? pLoadInfo->viewportWidth : viewport["width"],
			std::string(viewport["height"]).compare("VIEWPORT_HEIGHT") == 0 ? pLoadInfo->viewportHeight : viewport["height"],
			viewport["minDepth"],
			viewport["maxDepth"]
			});
	}
	for (auto& scissor : pipeline_grpf_json["viewportState"]["scissors"])
	{
		rpCI.viewportState.scissors.push_back({
			{
				scissor["x"],
				scissor["y"]
			},
			{
				(uint32_t)(std::string(scissor["width"]).compare("VIEWPORT_WIDTH") == 0 ? pLoadInfo->viewportWidth : scissor["width"]),
				(uint32_t)(std::string(scissor["height"]).compare("VIEWPORT_HEIGHT") == 0 ? pLoadInfo->viewportHeight : scissor["height"]),
			}
			});
	}

	//RasterisationState
	rpCI.rasterisationState.depthClampEnable = pipeline_grpf_json["rasterisationState"]["depthClampEnable"];
	rpCI.rasterisationState.rasteriserDiscardEnable = pipeline_grpf_json["rasterisationState"]["depthClampEnable"];
	rpCI.rasterisationState.polygonMode = PolygonModeStrings[pipeline_grpf_json["rasterisationState"]["polygonMode"]];
	rpCI.rasterisationState.cullMode = CullModeBitStrings[pipeline_grpf_json["rasterisationState"]["cullMode"]];
	rpCI.rasterisationState.frontFace = FrontFaceStrings[pipeline_grpf_json["rasterisationState"]["frontFace"]];
	rpCI.rasterisationState.depthBiasEnable = pipeline_grpf_json["rasterisationState"]["depthBiasEnable"];
	rpCI.rasterisationState.depthBiasConstantFactor = pipeline_grpf_json["rasterisationState"]["depthBiasConstantFactor"];
	rpCI.rasterisationState.depthBiasClamp = pipeline_grpf_json["rasterisationState"]["depthBiasClamp"];
	rpCI.rasterisationState.depthBiasSlopeFactor = pipeline_grpf_json["rasterisationState"]["depthBiasSlopeFactor"];
	rpCI.rasterisationState.lineWidth = pipeline_grpf_json["rasterisationState"]["lineWidth"];

	//MultisampleState
	rpCI.multisampleState.rasterisationSamples = std::string(pipeline_grpf_json["multisampleState"]["rasterisationSamples"]).compare("FRAMEBUFFER_SAMPLE_COUNT") == 0 ? pLoadInfo->samples : SampleCountBitStrings[pipeline_grpf_json["multisampleState"]["rasterisationSamples"]];
	rpCI.multisampleState.sampleShadingEnable = pipeline_grpf_json["multisampleState"]["sampleShadingEnable"];
	rpCI.multisampleState.minSampleShading = pipeline_grpf_json["multisampleState"]["minSampleShading"];
	rpCI.multisampleState.alphaToCoverageEnable = pipeline_grpf_json["multisampleState"]["alphaToCoverageEnable"];
	rpCI.multisampleState.alphaToOneEnable = pipeline_grpf_json["multisampleState"]["alphaToOneEnable"];

	//DepthStencilState
	rpCI.depthStencilState.depthTestEnable = pipeline_grpf_json["depthStencilState"]["depthTestEnable"];
	rpCI.depthStencilState.depthWriteEnable = pipeline_grpf_json["depthStencilState"]["depthWriteEnable"];
	rpCI.depthStencilState.depthCompareOp = CompareOpStrings[pipeline_grpf_json["depthStencilState"]["depthCompareOp"]];
	rpCI.depthStencilState.depthBoundsTestEnable = pipeline_grpf_json["depthStencilState"]["depthBoundsTestEnable"];
	rpCI.depthStencilState.stencilTestEnable = pipeline_grpf_json["depthStencilState"]["stencilTestEnable"];
	if (rpCI.depthStencilState.stencilTestEnable)
	{
		rpCI.depthStencilState.front.failOp = StencilOpStrings[pipeline_grpf_json["depthStencilState"]["front"]["failOp"]];
		rpCI.depthStencilState.front.passOp = StencilOpStrings[pipeline_grpf_json["depthStencilState"]["front"]["passOp"]];
		rpCI.depthStencilState.front.depthFailOp = StencilOpStrings[pipeline_grpf_json["depthStencilState"]["front"]["depthFailOp"]];
		rpCI.depthStencilState.front.compareOp = CompareOpStrings[pipeline_grpf_json["depthStencilState"]["front"]["compareOp"]];
		rpCI.depthStencilState.front.compareMask = pipeline_grpf_json["depthStencilState"]["front"]["compareMask"];
		rpCI.depthStencilState.front.writeMask = pipeline_grpf_json["depthStencilState"]["front"]["writeMask"];
		rpCI.depthStencilState.front.reference = pipeline_grpf_json["depthStencilState"]["front"]["reference"];
		rpCI.depthStencilState.back.failOp = StencilOpStrings[pipeline_grpf_json["depthStencilState"]["back"]["failOp"]];
		rpCI.depthStencilState.back.passOp = StencilOpStrings[pipeline_grpf_json["depthStencilState"]["back"]["passOp"]];
		rpCI.depthStencilState.back.depthFailOp = StencilOpStrings[pipeline_grpf_json["depthStencilState"]["back"]["depthFailOp"]];
		rpCI.depthStencilState.back.compareOp = CompareOpStrings[pipeline_grpf_json["depthStencilState"]["back"]["compareOp"]];
		rpCI.depthStencilState.back.compareMask = pipeline_grpf_json["depthStencilState"]["back"]["compareMask"];
		rpCI.depthStencilState.back.writeMask = pipeline_grpf_json["depthStencilState"]["back"]["writeMask"];
		rpCI.depthStencilState.back.reference = pipeline_grpf_json["depthStencilState"]["back"]["reference"];
	}
	else
	{
		rpCI.depthStencilState.front = {};
		rpCI.depthStencilState.back = {};
	}
	rpCI.depthStencilState.minDepthBounds = pipeline_grpf_json["depthStencilState"]["minDepthBounds"];
	rpCI.depthStencilState.maxDepthBounds = pipeline_grpf_json["depthStencilState"]["maxDepthBounds"];

	//ColourBlendState
	rpCI.colourBlendState.logicOpEnable = pipeline_grpf_json["colourBlendState"]["logicOpEnable"];
	rpCI.colourBlendState.logicOp = LogicOpStrings[pipeline_grpf_json["colourBlendState"]["logicOp"]];
	for (auto& attachment : pipeline_grpf_json["colourBlendState"]["attachments"])
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
	rpCI.colourBlendState.blendConstants[0] = pipeline_grpf_json["colourBlendState"]["blendConstants"][0];
	rpCI.colourBlendState.blendConstants[1] = pipeline_grpf_json["colourBlendState"]["blendConstants"][1];
	rpCI.colourBlendState.blendConstants[2] = pipeline_grpf_json["colourBlendState"]["blendConstants"][2];
	rpCI.colourBlendState.blendConstants[3] = pipeline_grpf_json["colourBlendState"]["blendConstants"][3];

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
			if (arc::FileLastWriteTime(shader->GetCreateInfo().binaryFilepath) < arc::FileLastWriteTime(shader->GetCreateInfo().recompileArguments.hlslFilepath))
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
