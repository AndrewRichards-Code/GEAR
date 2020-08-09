#include "gear_core_common.h"
#include "Renderer.h"
#include "Core/EnumStringMaps.h"

using namespace gear;
using namespace graphics;
using namespace objects;

using namespace miru;
using namespace miru::crossplatform;

Renderer::Renderer(const miru::Ref<Context>& context)
{
	//Renderer and Transfer CmdPools and CmdBuffers
	m_CmdPoolCI.debugName = "GEAR_CORE_CommandPool_Renderer";
	m_CmdPoolCI.pContext = context;
	m_CmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_CmdPoolCI.queueFamilyIndex = 0;
	m_CmdPool = CommandPool::Create(&m_CmdPoolCI);

	m_CmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_Renderer";
	m_CmdBufferCI.pCommandPool = m_CmdPool;
	m_CmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_CmdBufferCI.commandBufferCount = 3;
	m_CmdBufferCI.allocateNewCommandPoolPerBuffer = GraphicsAPI::IsD3D12();
	m_CmdBuffer = CommandBuffer::Create(&m_CmdBufferCI);

	m_TransCmdPoolCI.debugName = "GEAR_CORE_CommandPool_Renderer_Transfer";
	m_TransCmdPoolCI.pContext = context;
	m_TransCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_TransCmdPoolCI.queueFamilyIndex = 2;
	m_TransCmdPool = CommandPool::Create(&m_TransCmdPoolCI);

	m_TransCmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_Renderer_Transfer";
	m_TransCmdBufferCI.pCommandPool = m_TransCmdPool;
	m_TransCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_TransCmdBufferCI.commandBufferCount = 1;
	m_TransCmdBufferCI.allocateNewCommandPoolPerBuffer = GraphicsAPI::IsD3D12();
	m_TransCmdBuffer = CommandBuffer::Create(&m_TransCmdBufferCI);

	m_Context = context;
	m_Device = context->GetDevice();
	
	//Present Synchronisation
	m_DrawFenceCI.debugName = "GEAR_CORE_Fence_Renderer_Draw";
	m_DrawFenceCI.device = m_Device;
	m_DrawFenceCI.signaled = true;
	m_DrawFenceCI.timeout = UINT64_MAX;
	m_DrawFences = { Fence::Create(&m_DrawFenceCI), Fence::Create(&m_DrawFenceCI) };

	m_AcquireSemaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Acquire";
	m_AcquireSemaphoreCI.device = m_Device;
	m_AcquireSemaphores = { Semaphore::Create(&m_AcquireSemaphoreCI), Semaphore::Create(&m_AcquireSemaphoreCI) };

	m_SubmitSemaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Submit";
	m_SubmitSemaphoreCI.device = m_Device;
	m_SubmitSemaphores = { Semaphore::Create(&m_SubmitSemaphoreCI), Semaphore::Create(&m_SubmitSemaphoreCI) };

}

Renderer::~Renderer()
{
	m_Context->DeviceWaitIdle();
	ClearupRenderPipelines();
}

void Renderer::InitialiseRenderPipelines(const std::vector<std::string>& filepaths, float viewportWidth, float viewportHeight, const miru::Ref<RenderPass>& renderPass)
{
	for (auto& filepath : filepaths)
		AddRenderPipeline(filepath, viewportWidth, viewportHeight, renderPass);
}

void Renderer::ClearupRenderPipelines()
{
	for (auto& renderPipeline : m_RenderPipelines)
	{
		for (auto& shaderCI : renderPipeline.second->m_CI.shaderCreateInfo)
		{
			free((void*)shaderCI.debugName);
			free((void*)shaderCI.entryPoint);
			free((void*)shaderCI.binaryFilepath);
			free((void*)shaderCI.recompileArguments.mscDirectory);
			free((void*)shaderCI.recompileArguments.hlslFilepath);
			free((void*)shaderCI.recompileArguments.outputDirectory);

			for(auto& includeDir : shaderCI.recompileArguments.includeDirectories)
				free((void*)includeDir);
			for(auto& macro : shaderCI.recompileArguments.macros)
				free((void*)macro);

			free((void*)shaderCI.recompileArguments.dxcLocation);
			free((void*)shaderCI.recompileArguments.glslangLocation);
			free((void*)shaderCI.recompileArguments.additioalArguments);
		}
	}
}

void Renderer::SubmitModel(const gear::Ref<Model>& obj)
{
	m_RenderQueue.push_back(obj);
}

void Renderer::Flush()
{
	Semaphore::CreateInfo transSemaphoreCI = { "GEAR_CORE_Semaphore_RenderTransfer", m_TransCmdPoolCI.pContext->GetDevice() };
	Ref<Semaphore> transfer = Semaphore::Create(&transSemaphoreCI);
	Fence::CreateInfo transFenceCI = { "GEAR_CORE_Fence_RenderTransfer", m_TransCmdPoolCI.pContext->GetDevice(), false, UINT64_MAX };
	Ref<Fence> transferFence = Fence::Create(&transFenceCI);

	uint32_t uploadedTexturesCount = 0;

	//Upload CmdBufer
	{
		m_TransCmdBuffer->Reset(0, false);
		m_TransCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
		m_Camera->GetUB()->Upload(m_TransCmdBuffer, 0, true);
		m_Lights[0]->GetUB()->Upload(m_TransCmdBuffer, 0);

		std::vector<Ref<Barrier>> initialBarrier;
		for (auto& model : m_RenderQueue)
		{
			for (auto& vb : model->GetMesh()->GetVertexBuffers())
				vb->Upload(m_TransCmdBuffer, 0);
			for (auto& ib : model->GetMesh()->GetIndexBuffers())
				ib->Upload(m_TransCmdBuffer, 0);

			model->GetUB()->Upload(m_TransCmdBuffer, 0);
			model->GetMesh()->GetMaterials()[0]->GetUB()->Upload(m_TransCmdBuffer, 0);

			for (auto& material : model->GetMesh()->GetMaterials())
			{
				for (auto& texture : material->GetTextures())
				{
					texture.second->GetInitialTransition(initialBarrier);
				}
			}
		}

		m_TransCmdBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, initialBarrier);

		for (auto& model : m_RenderQueue)
		{
			for (auto& material : model->GetMesh()->GetMaterials())
			{
				for (auto& texture : material->GetTextures())
				{
					texture.second->Upload(m_TransCmdBuffer, 0);
					uploadedTexturesCount++;
				}
			}
		}

		m_TransCmdBuffer->End(0);
	}
	m_TransCmdBuffer->Submit({ 0 }, {}, { transfer }, PipelineStageBit::TRANSFER_BIT, nullptr);
	{
		m_CmdBuffer->Reset(2, false);
		m_CmdBuffer->Begin(2, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
		std::vector<Ref<Barrier>> finalBarrier;
		finalBarrier.reserve(uploadedTexturesCount);
		for (auto& model : m_RenderQueue)
		{
			for (auto& material : model->GetMesh()->GetMaterials())
			{
				for (auto& texture : material->GetTextures())
				{
					texture.second->GetFinalTransition(finalBarrier);
				}
			}
		}

		m_CmdBuffer->PipelineBarrier(2, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, DependencyBit::NONE_BIT, finalBarrier);
		m_CmdBuffer->End(2);
	}
	m_CmdBuffer->Submit({ 2 }, { transfer }, {}, PipelineStageBit::TRANSFER_BIT, transferFence);

	transferFence->Wait();

	if(!builtDescPoolsAndSets)
	{
		/*//Build DescriptorPools and Sets
		m_DescPoolCI.debugName = "GEAR_CORE_DescPool_Renderer";
		m_DescPoolCI.device = m_TransCmdPoolCI.pContext->GetDevice();
		m_DescPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, std::max(uploadedTexturesCount, (uint32_t)m_RenderQueue.size())}, {DescriptorType::UNIFORM_BUFFER, (uint32_t)m_RenderQueue.size() + 2} };
		m_DescPoolCI.maxSets = (uint32_t)m_RenderQueue.size() + 3;
		m_DescPool = DescriptorPool::Create(&m_DescPoolCI);
	
		DescriptorSetLayout::CreateInfo descSetLayoutCI;
		descSetLayoutCI.debugName = "GEAR_CORE_DescSetLayout_Renderer";
		descSetLayoutCI.device = m_TransCmdPoolCI.pContext->GetDevice();
		descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT} };
		m_DescSetLayouts.push_back(DescriptorSetLayout::Create(&descSetLayoutCI));
		descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT}, {1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT} };
		m_DescSetLayouts.push_back(DescriptorSetLayout::Create(&descSetLayoutCI));
		descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::FRAGMENT_BIT}, {1, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::FRAGMENT_BIT} };
		m_DescSetLayouts.push_back(DescriptorSetLayout::Create(&descSetLayoutCI));
	
		m_DescSetCI.debugName = "GEAR_CORE_DescSet_Renderer";
		m_DescSetCI.pDescriptorPool = m_DescPool;
		m_DescSetCI.pDescriptorSetLayouts = { m_DescSetLayouts[0] };
		m_DescSetCamera = DescriptorSet::Create(&m_DescSetCI);
		m_DescSetCamera->AddBuffer(0, 0, { { m_Camera->GetUB()->GetBufferView() } });
		m_DescSetCamera->Update();
	
		m_DescSetCI.pDescriptorSetLayouts = { m_DescSetLayouts[2] };
		m_DescSetLight = DescriptorSet::Create(&m_DescSetCI);
		m_DescSetLight->AddBuffer(0, 0, { { m_Lights[0]->GetUB()->GetBufferView() } });
		m_DescSetLight->Update();
	
		for (auto& obj : m_RenderQueue)
		{
			auto& materialTextures = obj->GetMesh()->GetMaterials()[0]->GetTextures();
			gear::Ref<Texture> texture = nullptr;
			if(!materialTextures.empty())
				materialTextures.begin()->second;

			m_DescSetCI.pDescriptorSetLayouts = { m_DescSetLayouts[1] };
			m_DescSetObj[obj] = DescriptorSet::Create(&m_DescSetCI);
			if(texture)
				m_DescSetObj[obj]->AddImage(0, 1, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			m_DescSetObj[obj]->AddBuffer(0, 0, { { obj->GetUB()->GetBufferView() } });
			m_DescSetObj[obj]->Update();
		}*/
		bool cameraPoolSize = false, lightPoolSize = false;
		std::map<DescriptorType, uint32_t> poolSizesMap;
		for (auto& model : m_RenderQueue)
		{
			const miru::Ref<Pipeline>& pipeline = m_RenderPipelines[model->GetPipelineName()]->GetPipeline();
			const std::vector<miru::Ref<DescriptorSetLayout>>& descriptorSetLayouts = m_RenderPipelines[model->GetPipelineName()]->GetDescriptorSetLayouts();

			const DescriptorSetLayout::CreateInfo& cameraDescSetLayoutCI = descriptorSetLayouts[0]->GetCreateInfo();
			const DescriptorSetLayout::CreateInfo& modelMaterialDescSetLayoutCI = descriptorSetLayouts[1]->GetCreateInfo();
			const DescriptorSetLayout::CreateInfo& lightDescSetLayoutCI = descriptorSetLayouts[2]->GetCreateInfo();

			auto AddToPoolSizeMap = [&](const DescriptorSetLayout::CreateInfo& descSetLayoutCI) -> void
			{
				for (auto& descSetLayout : descSetLayoutCI.descriptorSetLayoutBinding)
				{
					uint32_t& descCount = poolSizesMap[descSetLayout.type];
					descCount += descSetLayout.descriptorCount;
				}
			};

			if (!cameraPoolSize)
			{
				AddToPoolSizeMap(cameraDescSetLayoutCI);
				cameraPoolSize = true;
			}
			if (!lightPoolSize)
			{
				AddToPoolSizeMap(lightDescSetLayoutCI);
				lightPoolSize = true;
			}
			AddToPoolSizeMap(modelMaterialDescSetLayoutCI);
		}
		m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_Renderer";
		m_DescPoolCI.device = m_Device;
		for (auto& poolSize : poolSizesMap)
			m_DescPoolCI.poolSizes.push_back({ poolSize.first, poolSize.second });
		m_DescPoolCI.maxSets = static_cast<uint32_t>(m_RenderQueue.size() + 2);
		m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

		for (auto& model : m_RenderQueue)
		{
			const miru::Ref<Pipeline>& pipeline = m_RenderPipelines[model->GetPipelineName()]->GetPipeline();
			const std::vector<miru::Ref<DescriptorSetLayout>>& descriptorSetLayouts = m_RenderPipelines[model->GetPipelineName()]->GetDescriptorSetLayouts();

			const miru::Ref<DescriptorSetLayout>& cameraDescSetLayout = descriptorSetLayouts[0];
			const miru::Ref<DescriptorSetLayout>& modelMaterialDescSetLayout = descriptorSetLayouts[1];
			const miru::Ref<DescriptorSetLayout>& lightDescLayout = descriptorSetLayouts[2];

			if (!m_DescSetCamera)
			{
				DescriptorSet::CreateInfo cameraDescSetCI;
				cameraDescSetCI.debugName = "GEAR_CORE_DescriptorSet_Camera";
				cameraDescSetCI.pDescriptorPool = m_DescPool;
				cameraDescSetCI.pDescriptorSetLayouts = { cameraDescSetLayout };
				m_DescSetCamera = DescriptorSet::Create(&cameraDescSetCI);
				m_DescSetCamera->AddBuffer(0, 0, { { m_Camera->GetUB()->GetBufferView() } });
				m_DescSetCamera->Update();
			}

			if (!m_DescSetLight)
			{
				DescriptorSet::CreateInfo lightDescSetCI;
				lightDescSetCI.debugName = "GEAR_CORE_DescriptorSet_Light";
				lightDescSetCI.pDescriptorPool = m_DescPool;
				lightDescSetCI.pDescriptorSetLayouts = { lightDescLayout };
				m_DescSetLight = DescriptorSet::Create(&lightDescSetCI); m_DescSetLight->AddBuffer(0, 0, { { m_Lights[0]->GetUB()->GetBufferView() } });
				m_DescSetLight->Update();
			}

			DescriptorSet::CreateInfo modelMaterialSetCI;
			m_DescSetModelMaterialDebugName[model] = "GEAR_CORE_DescriptorSet_ModelMaterial: " + model->GetDebugName();
			modelMaterialSetCI.debugName = m_DescSetModelMaterialDebugName[model].c_str();
			modelMaterialSetCI.pDescriptorPool = m_DescPool;
			modelMaterialSetCI.pDescriptorSetLayouts = { modelMaterialDescSetLayout };
			m_DescSetModelMaterials[model] = DescriptorSet::Create(&modelMaterialSetCI);

			m_DescSetModelMaterials[model]->AddBuffer(0, 0, { { model->GetUB()->GetBufferView() } });
			m_DescSetModelMaterials[model]->AddBuffer(0, 1, { { model->GetMesh()->GetMaterials()[0]->GetUB()->GetBufferView() } });
			
			for (auto& materialTexture : model->GetMesh()->GetMaterials()[0]->GetTextures()) //TODO: Deal with all materials
			{
				const Material::TextureType& type = materialTexture.first;
				const gear::Ref<Texture>& material = materialTexture.second;

				switch (type)
				{
				case Material::TextureType::NORMAL:
					m_DescSetModelMaterials[model]->AddImage(0, 2, { {material->GetTextureSampler(), material->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				case Material::TextureType::ALBEDO:
					m_DescSetModelMaterials[model]->AddImage(0, 3, { {material->GetTextureSampler(), material->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				case Material::TextureType::METALLIC:
					m_DescSetModelMaterials[model]->AddImage(0, 4, { {material->GetTextureSampler(), material->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				case Material::TextureType::ROUGHNESS:
					m_DescSetModelMaterials[model]->AddImage(0, 5, { {material->GetTextureSampler(), material->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				case Material::TextureType::AMBIENT_OCCLUSION:
					m_DescSetModelMaterials[model]->AddImage(0, 6, { {material->GetTextureSampler(), material->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				case Material::TextureType::EMISSIVE:
					m_DescSetModelMaterials[model]->AddImage(0, 7, { {material->GetTextureSampler(), material->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				default:
					continue;
				}
			}
			m_DescSetModelMaterials[model]->Update();
		}

		builtDescPoolsAndSets = true;
	}

	//Record Present CmdBuffers
	m_DrawFences[m_FrameIndex]->Wait();
	{
		m_CmdBuffer->Reset(m_FrameIndex, false);
		m_CmdBuffer->Begin(m_FrameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
		m_CmdBuffer->BeginRenderPass(m_FrameIndex, m_Framebuffers[m_FrameIndex], { {0.25f, 0.25f, 0.25f, 1.0f}, {1.0f, 0} });

		for (auto& model : m_RenderQueue)
		{
			const miru::Ref<Pipeline>& pipeline = m_RenderPipelines[model->GetPipelineName()]->GetPipeline();
			const std::vector<miru::Ref<DescriptorSetLayout>>& descriptorSetLayouts = m_RenderPipelines[model->GetPipelineName()]->GetDescriptorSetLayouts();

			m_CmdBuffer->BindPipeline(m_FrameIndex, pipeline);

			std::vector<Ref<DescriptorSet>> bindDescriptorSets;
			for (size_t descCount = 0; descCount < descriptorSetLayouts.size(); descCount++)
			{
				switch (descCount)
				{
				case 0:
					bindDescriptorSets.push_back(m_DescSetCamera); continue;
				case 1:
					bindDescriptorSets.push_back(m_DescSetModelMaterials[model]); continue;
				case 2:
					bindDescriptorSets.push_back(m_DescSetLight); continue;
				default:
					continue;
				}
			}
			m_CmdBuffer->BindDescriptorSets(m_FrameIndex, bindDescriptorSets, pipeline);

			for (size_t i = 0; i < model->GetMesh()->GetVertexBuffers().size(); i++)
			{
				m_CmdBuffer->BindVertexBuffers(m_FrameIndex, { model->GetMesh()->GetVertexBuffers()[i]->GetVertexBufferView() });
				m_CmdBuffer->BindIndexBuffer(m_FrameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetIndexBufferView());

				m_CmdBuffer->DrawIndexed(m_FrameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetCount());
			}
		}
		m_CmdBuffer->EndRenderPass(m_FrameIndex);
		m_CmdBuffer->End(m_FrameIndex);
	}

	m_RenderQueue.clear();
}

void Renderer::Present(const miru::Ref<Swapchain>& swapchain, bool& windowResize)
{
	m_CmdBuffer->Present({ 0, 1 }, swapchain, m_DrawFences, m_AcquireSemaphores, m_SubmitSemaphores, windowResize);
	m_FrameIndex = (m_FrameIndex + 1) % swapchain->GetCreateInfo().swapchainCount;
	m_FrameCount++;
}

void Renderer::AddRenderPipeline(const std::string& filepath, float viewportWidth, float viewportHeight, const miru::Ref<RenderPass>& renderPass)
{
	using namespace nlohmann;

	std::string mscDirectory = "../GEAR_CORE/dep/MIRU/MIRU_SHADER_COMPILER/exe/x64/";
#if _DEBUG
	mscDirectory += "Debug";
#else
	mscDirectory += "Release";
#endif

	json pipeline_grpf_json;
	std::ifstream pipeline_grpf(filepath, std::ios::binary);
	if (pipeline_grpf.is_open())
	{
		pipeline_grpf >> pipeline_grpf_json;
	}
	else
	{
		GEAR_WARN(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_NO_FILE, "WARNING: gear::graphics::Renderer: Unable to open grpf.json file.");
	}

	if (pipeline_grpf_json.empty())
	{
		GEAR_WARN(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_LOAD_FAILED, "WARNING: gear::graphics::Renderer: grpf.json file is not valid.");
	}

	std::string fileType = pipeline_grpf_json["fileType"];
	if (fileType.compare("GEAR_RENDER_PIPELINE_FILE") != 0)
	{
		GEAR_WARN(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_NOT_SUPPORTED, "WARNING: gear::graphics::Renderer: grpf.json file is not valid.");
	}

	auto dup_string = [](const std::string& value) -> const char*
	{
		if (value.size())
			return _strdup(value.c_str());
		else
			return nullptr;
	};

	RenderPipeline::CreateInfo rpCI;
	rpCI.debugName = dup_string(pipeline_grpf_json["debugName"]);

	//Shaders
	for (auto& shader : pipeline_grpf_json["shaders"])
	{
		Shader::CreateInfo shaderCI;
		shaderCI.debugName = dup_string(shader["debugName"]);
		shaderCI.device = m_Device;
		shaderCI.stage = ShaderStageBitStrings[shader["stage"]];
		shaderCI.entryPoint = dup_string(shader["entryPoint"]);
		shaderCI.binaryFilepath = dup_string(shader["binaryFilepath"]);
		shaderCI.binaryCode = {};

		auto& recompileArgs = shader["recompileArguments"];
		shaderCI.recompileArguments.mscDirectory = dup_string(recompileArgs["mscDirectory"]);
		shaderCI.recompileArguments.hlslFilepath = dup_string(recompileArgs["hlslFilepath"]);
		shaderCI.recompileArguments.outputDirectory = dup_string(recompileArgs["outputDirectory"]);
		for (auto& includeDir : recompileArgs["includeDirectories"])
			shaderCI.recompileArguments.includeDirectories.push_back(dup_string(includeDir));
		shaderCI.recompileArguments.entryPoint = dup_string(recompileArgs["entryPoint"]);
		shaderCI.recompileArguments.shaderModel = dup_string(recompileArgs["shaderModel"]);
		for (auto& macro : recompileArgs["macros"])
			shaderCI.recompileArguments.macros.push_back(dup_string(macro));
		shaderCI.recompileArguments.cso = recompileArgs["cso"];
		shaderCI.recompileArguments.spv = recompileArgs["spv"];
		shaderCI.recompileArguments.dxcLocation = dup_string(recompileArgs["dxcLocation"]);
		shaderCI.recompileArguments.glslangLocation = dup_string(recompileArgs["glslangLocation"]);
		shaderCI.recompileArguments.additioalArguments = dup_string(recompileArgs["additioalArguments"]);
		shaderCI.recompileArguments.nologo = recompileArgs["nologo"];
		shaderCI.recompileArguments.nooutput = recompileArgs["nooutput"];

		rpCI.shaderCreateInfo.push_back(shaderCI);
	}

	//ViewportState
	for (auto& viewport : pipeline_grpf_json["viewportState"]["viewports"])
	{
		rpCI.viewportState.viewports.push_back({
			viewport["x"],
			viewport["y"],
			std::string(viewport["width"]).compare("VIEWPORT_WIDTH") == 0 ? viewportWidth : viewport["width"],
			std::string(viewport["height"]).compare("VIEWPORT_HEIGHT") == 0 ? viewportHeight : viewport["height"],
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
				(uint32_t)(std::string(scissor["width"]).compare("VIEWPORT_WIDTH") == 0 ? viewportWidth : scissor["width"]),
				(uint32_t)(std::string(scissor["height"]).compare("VIEWPORT_HEIGHT") == 0 ? viewportHeight : scissor["height"]),
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
	rpCI.multisampleState.rasterisationSamples = SampleCountBitStrings[pipeline_grpf_json["multisampleState"]["rasterisationSamples"]];
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
	rpCI.renderPass = renderPass;
	rpCI.subpassIndex = 0;

	m_RenderPipelines[rpCI.debugName] = gear::CreateRef<graphics::RenderPipeline>(&rpCI);
}