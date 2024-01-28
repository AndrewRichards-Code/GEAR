#include "gear_core_common.h"

#include "Graphics/RenderPipeline.h"

#include "Core/JsonFileHelper.h"
#include "Utils/ModelLoader.h"

#include <regex>

using namespace gear;
using namespace core;
using namespace graphics;

using namespace miru;
using namespace base;

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

	std::filesystem::path finalFilePath = std::filesystem::path(SOURCE_DIR) / "GEAR_CORE" / std::filesystem::path(pLoadInfo->filepath);
	
	json pipeline_grpf;
	core::LoadJsonFile(finalFilePath.string(), ".grpf", "GEAR_RENDER_PIPELINE_FILE", pipeline_grpf);
	
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
			shaderCI.stageAndEntryPoints.push_back({magic_enum::enum_cast<Shader::StageBit>(std::string(stageAndEntryPoint["stage"])).value(), stageAndEntryPoint["entryPoint"]});
		}
		shaderCI.binaryFilepath = std::regex_replace(std::string(shader["binaryFilepath"]), std::regex("\\$BUILD_DIR"), BUILD_DIR);
		shaderCI.binaryCode = {};

		json& recompileArgs = shader["recompileArguments"];
		const std::string& rafFilepath = recompileArgs["rafFilepath"];
		const size_t& index = recompileArgs["index"];

		const std::string& filepath = std::regex_replace(rafFilepath, std::regex("\\$SOURCE_DIR"), SOURCE_DIR);
		std::unordered_map<std::string, std::string> rafEnvironmentVariables =
		{
			{"$BUILD_DIR", BUILD_DIR},
			{"$SOURCE_DIR", SOURCE_DIR},
		};
		const std::vector<base::Shader::CompileArguments> compileArguments
			= base::Shader::LoadCompileArgumentsFromFile(
				filepath, rafEnvironmentVariables);

		shaderCI.recompileArguments = compileArguments[index];

		rpCI.shaderCreateInfo.push_back(shaderCI);
	}

	//Compute Pipelines are completed here.
	if (rpCI.shaderCreateInfo.size() == 1 
		&& rpCI.shaderCreateInfo.back().stageAndEntryPoints.size() == 1 
		&& rpCI.shaderCreateInfo.back().stageAndEntryPoints.back().first == Shader::StageBit::COMPUTE_BIT)
	{
		*this = RenderPipeline(&rpCI);
		return;
	}

	//InputAssembly
	json& inputAssemblyState = pipeline_grpf["inputAssemblyState"];
	rpCI.inputAssemblyState.topology = magic_enum::enum_cast<PrimitiveTopology>(std::string(inputAssemblyState["topology"])).value();
	rpCI.inputAssemblyState.primitiveRestartEnable = inputAssemblyState["primitiveRestartEnable"];

	//ViewportState
	json& viewportState = pipeline_grpf["viewportState"];
	for (auto& viewport : viewportState["viewports"])
	{
		rpCI.viewportState.viewports.push_back({
			viewport["x"],
			viewport["y"],
			viewport["width"],
			viewport["height"],
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
				scissor["width"],
				scissor["height"]
			}
			});
	}

	//RasterisationState
	json& rasterisationState = pipeline_grpf["rasterisationState"];
	rpCI.rasterisationState.depthClampEnable = rasterisationState["depthClampEnable"];
	rpCI.rasterisationState.rasteriserDiscardEnable = rasterisationState["depthClampEnable"];
	rpCI.rasterisationState.polygonMode = magic_enum::enum_cast<PolygonMode>(std::string(rasterisationState["polygonMode"])).value();
	rpCI.rasterisationState.cullMode = magic_enum::enum_cast<CullModeBit>(std::string(rasterisationState["cullMode"])).value();
	rpCI.rasterisationState.frontFace = magic_enum::enum_cast<FrontFace>(std::string(rasterisationState["frontFace"])).value();
	rpCI.rasterisationState.depthBiasEnable = rasterisationState["depthBiasEnable"];
	rpCI.rasterisationState.depthBiasConstantFactor = rasterisationState["depthBiasConstantFactor"];
	rpCI.rasterisationState.depthBiasClamp = rasterisationState["depthBiasClamp"];
	rpCI.rasterisationState.depthBiasSlopeFactor = rasterisationState["depthBiasSlopeFactor"];
	rpCI.rasterisationState.lineWidth = rasterisationState["lineWidth"];

	//MultisampleState
	json& multisampleState = pipeline_grpf["multisampleState"];
	rpCI.multisampleState.rasterisationSamples = std::string(multisampleState["rasterisationSamples"]).compare("FRAMEBUFFER_SAMPLE_COUNT") == 0 
		? pLoadInfo->samples : magic_enum::enum_cast<Image::SampleCountBit>(std::string(multisampleState["rasterisationSamples"])).value();
	rpCI.multisampleState.sampleShadingEnable = multisampleState["sampleShadingEnable"];
	rpCI.multisampleState.minSampleShading = multisampleState["minSampleShading"];
	rpCI.multisampleState.sampleMask = multisampleState["sampleMask"];
	rpCI.multisampleState.alphaToCoverageEnable = multisampleState["alphaToCoverageEnable"];
	rpCI.multisampleState.alphaToOneEnable = multisampleState["alphaToOneEnable"];

	//DepthStencilState
	json& depthStencilState = pipeline_grpf["depthStencilState"];
	rpCI.depthStencilState.depthTestEnable = depthStencilState["depthTestEnable"];
	rpCI.depthStencilState.depthWriteEnable = depthStencilState["depthWriteEnable"];
	rpCI.depthStencilState.depthCompareOp = magic_enum::enum_cast<CompareOp>(std::string(depthStencilState["depthCompareOp"])).value();
	rpCI.depthStencilState.depthBoundsTestEnable = depthStencilState["depthBoundsTestEnable"];
	rpCI.depthStencilState.stencilTestEnable = depthStencilState["stencilTestEnable"];
	if (rpCI.depthStencilState.stencilTestEnable)
	{
		json& front = depthStencilState["front"];
		rpCI.depthStencilState.front.failOp = magic_enum::enum_cast<StencilOp>(std::string(front["failOp"])).value();
		rpCI.depthStencilState.front.passOp = magic_enum::enum_cast<StencilOp>(std::string(front["passOp"])).value();
		rpCI.depthStencilState.front.depthFailOp = magic_enum::enum_cast<StencilOp>(std::string(front["depthFailOp"])).value();
		rpCI.depthStencilState.front.compareOp = magic_enum::enum_cast<CompareOp>(std::string(front["compareOp"])).value();
		rpCI.depthStencilState.front.compareMask = front["compareMask"];
		rpCI.depthStencilState.front.writeMask = front["writeMask"];
		rpCI.depthStencilState.front.reference = front["reference"];
		json& back = depthStencilState["back"];
		rpCI.depthStencilState.back.failOp = magic_enum::enum_cast<StencilOp>(std::string(back["failOp"])).value();
		rpCI.depthStencilState.back.passOp = magic_enum::enum_cast<StencilOp>(std::string(back["passOp"])).value();
		rpCI.depthStencilState.back.depthFailOp = magic_enum::enum_cast<StencilOp>(std::string(back["depthFailOp"])).value();
		rpCI.depthStencilState.back.compareOp = magic_enum::enum_cast<CompareOp>(std::string(back["compareOp"])).value();
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
	rpCI.colourBlendState.logicOp = magic_enum::enum_cast<LogicOp>(std::string(colourBlendState["logicOp"])).value();
	for (auto& attachment : colourBlendState["attachments"])
	{
		ColourComponentBit ccb = ColourComponentBit(0);
		for (auto& cc : attachment["colourWriteMask"])
			ccb |= magic_enum::enum_cast<ColourComponentBit>(std::string(cc)).value();

		rpCI.colourBlendState.attachments.push_back({
			attachment["blendEnable"],
			magic_enum::enum_cast<BlendFactor>(std::string(attachment["srcColourBlendFactor"])).value(),
			magic_enum::enum_cast<BlendFactor>(std::string(attachment["dstColourBlendFactor"])).value(),
			magic_enum::enum_cast<BlendOp>(std::string(attachment["colourBlendOp"])).value(),
			magic_enum::enum_cast<BlendFactor>(std::string(attachment["srcAlphaBlendFactor"])).value(),
			magic_enum::enum_cast<BlendFactor>(std::string(attachment["dstAlphaBlendFactor"])).value(),
			magic_enum::enum_cast<BlendOp>(std::string(attachment["alphaBlendOp"])).value(),
			ccb
			});
	}
	rpCI.colourBlendState.blendConstants[0] = colourBlendState["blendConstants"][0];
	rpCI.colourBlendState.blendConstants[1] = colourBlendState["blendConstants"][1];
	rpCI.colourBlendState.blendConstants[2] = colourBlendState["blendConstants"][2];
	rpCI.colourBlendState.blendConstants[3] = colourBlendState["blendConstants"][3];

	//Dynamic Rendering
	uint32_t viewMask = 0;
	if (pipeline_grpf.find("dynamicRendering") != pipeline_grpf.end())
		viewMask = pipeline_grpf["dynamicRendering"]["viewMask"];

	rpCI.dynamicRendering = { viewMask, pLoadInfo->colourAttachmentFormats, pLoadInfo->depthAttachmentFormat, Image::Format::UNKNOWN };

	*this = RenderPipeline(&rpCI);
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

			viads.push_back({ vsiad.location, 0, vsiad.vertexType, vsiad.offset, vsiad.semanticName.c_str() });
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

				for (auto& rbd : rbds.second)
				{
					uint32_t binding = rbd.second.binding;
					m_DescSetLayoutCIs[set].debugName = "GEAR_CORE_Pipeline_DescSetLayout_Set_" + std::to_string(set) + ": " + m_CI.debugName;;
					m_DescSetLayoutCIs[set].device = m_Device;
					m_DescSetLayoutCIs[set].descriptorSetLayoutBinding.push_back({
						rbd.second.binding,
						rbd.second.type,
						rbd.second.descriptorCount,
						rbd.second.stage });

					m_RBDs[set][binding] = rbd.second;
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
		m_PipelineCI.dynamicStates = { { DynamicState::VIEWPORT, DynamicState::SCISSOR } };
		m_PipelineCI.layout.descriptorSetLayouts = m_DescSetLayouts;
		m_PipelineCI.layout.pushConstantRanges = {};
		m_PipelineCI.renderPass = nullptr;
		m_PipelineCI.subpassIndex = 0;
		m_PipelineCI.dynamicRendering = m_CI.dynamicRendering;
		m_Pipeline = Pipeline::Create(&m_PipelineCI);
	}
	else
	{
		for (auto& rbds : m_Shaders[0]->GetRBDs())
		{
			uint32_t set = rbds.first;
			m_DescSetLayoutCIs.resize(std::max((size_t)set + 1, m_DescSetLayoutCIs.size()));

			for (auto& rbd : rbds.second)
			{
				uint32_t binding = rbd.second.binding;
				m_DescSetLayoutCIs[set].debugName = "GEAR_CORE_Pipeline_DescSetLayout_Set_" + std::to_string(set) + ": " + m_CI.debugName;
				m_DescSetLayoutCIs[set].device = m_Device;
				m_DescSetLayoutCIs[set].descriptorSetLayoutBinding.push_back({
					rbd.second.binding,
					rbd.second.type,
					rbd.second.descriptorCount,
					rbd.second.stage });

				m_RBDs[set][binding] = rbd.second;
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
		m_Pipeline = Pipeline::Create(&m_PipelineCI);
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
	m_PipelineCI.dynamicRendering = m_CI.dynamicRendering;
	m_Pipeline = Pipeline::Create(&m_PipelineCI);
}
