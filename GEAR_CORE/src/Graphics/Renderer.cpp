#include "gear_core_common.h"
#include "Renderer.h"
#include "Core/StringConversion.h"
#include "ImageProcessing.h"

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
	m_CmdPoolCI.queueType = CommandPool::QueueType::GRAPHICS;
	m_CmdPool = CommandPool::Create(&m_CmdPoolCI);

	m_CmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_Renderer";
	m_CmdBufferCI.pCommandPool = m_CmdPool;
	m_CmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_CmdBufferCI.commandBufferCount = 4;
	m_CmdBufferCI.allocateNewCommandPoolPerBuffer = GraphicsAPI::IsD3D12();
	m_CmdBuffer = CommandBuffer::Create(&m_CmdBufferCI);

	m_TransCmdPoolCI.debugName = "GEAR_CORE_CommandPool_Renderer_Transfer";
	m_TransCmdPoolCI.pContext = context;
	m_TransCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_TransCmdPoolCI.queueType = CommandPool::QueueType::TRANSFER;
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
}

void Renderer::InitialiseRenderPipelines(const std::vector<std::string>& filepaths, float viewportWidth, float viewportHeight, Image::SampleCountBit samples, const miru::Ref<RenderPass>& renderPass)
{
	RenderPipeline::LoadInfo renderPipelineLI;
	for (auto& filepath : filepaths)
	{
		//TODO: What if the Pipeline is already loaded?
		renderPipelineLI.device = m_Device;
		renderPipelineLI.filepath = filepath;
		renderPipelineLI.viewportWidth = viewportWidth;
		renderPipelineLI.viewportHeight = viewportHeight;
		renderPipelineLI.samples = samples;
		renderPipelineLI.renderPass = renderPass;
		renderPipelineLI.subpassIndex = 0;
		Ref<RenderPipeline> renderPipeline = CreateRef<RenderPipeline>(&renderPipelineLI);
		m_RenderPipelines[renderPipeline->m_CI.debugName] = renderPipeline;
	}
}

void Renderer::SubmitFramebuffer(const miru::Ref<miru::crossplatform::Framebuffer>* framebuffers)
{ 
	m_Framebuffers = framebuffers; 
}

void Renderer::SubmitCamera(gear::Ref<objects::Camera>& camera)
{ 
	m_Camera = camera; 
}

void Renderer::SubmitLights(std::vector<gear::Ref<objects::Light>>& lights)
{ 
	m_Lights = lights; 
}

void Renderer::SubmitSkybox(const gear::Ref<objects::Skybox>& skybox)
{ 
	m_Skybox = skybox; 
	SubmitModel(skybox->GetModel()); 
}

void Renderer::SubmitModel(const gear::Ref<Model>& obj)
{
	m_RenderQueue.push_back(obj);
}

void Renderer::Upload(bool forceUploadCamera, bool forceUploadLights, bool forceUploadSkybox, bool forceUploadMeshes)
{
	Semaphore::CreateInfo transSemaphoreCI = { "GEAR_CORE_Semaphore_RenderTransfer", m_TransCmdPoolCI.pContext->GetDevice() };
	Ref<Semaphore> transfer0 = Semaphore::Create(&transSemaphoreCI);
	Ref<Semaphore> transfer1 = Semaphore::Create(&transSemaphoreCI);

	Fence::CreateInfo fenceCI = { "GEAR_CORE_Fence_RenderTransfer", m_TransCmdPoolCI.pContext->GetDevice(), false, UINT64_MAX };
	Ref<Fence> preTransferGraphicsFence = Fence::Create(&fenceCI);
	Ref<Fence> transferFence = Fence::Create(&fenceCI);
	Ref<Fence> postTransferGraphicsFence = Fence::Create(&fenceCI);

	//Get Texture Barries and/or Reload Textures
	bool generateMipmaps = false;
	std::vector<Ref<Barrier>> textureUnknownToTransferDstBarrier;
	std::vector<Ref<Barrier>> textureShaderReadOnlyBarrierToTransferDst;
	std::vector<Ref<Barrier>> textureTransferDstToShaderReadOnlyBarrier;
	for (auto& model : m_RenderQueue)
	{
		for (auto& material : model->GetMesh()->GetMaterials())
		{
			for (auto& texture : material->GetTextures())
			{
				if (m_ReloadTextures)
				{
					auto reload_texture = [](gear::Ref<Texture> texture) { texture->Reload(); };
					std::async(std::launch::async, reload_texture, texture.second);
				}

				if (texture.second->m_GenerateMipMaps && !texture.second->m_Generated)
				{
					generateMipmaps = true;
				}

				if (!texture.second->m_TransitionUnknownToTransferDst)
				{
					texture.second->TransitionSubResources(textureUnknownToTransferDstBarrier,
						{ { Barrier::AccessBit::NONE_BIT, Barrier::AccessBit::TRANSFER_WRITE_BIT,
						Image::Layout::UNKNOWN, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
					texture.second->m_TransitionUnknownToTransferDst = true;
				}

				if (m_ReloadTextures && texture.second->m_TransitionTransferDstToShaderReadOnly)
				{
					texture.second->TransitionSubResources(textureShaderReadOnlyBarrierToTransferDst,
						{ { Barrier::AccessBit::SHADER_READ_BIT, Barrier::AccessBit::TRANSFER_WRITE_BIT,
						Image::Layout::SHADER_READ_ONLY_OPTIMAL, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
					texture.second->m_TransitionTransferDstToShaderReadOnly = false;
				}

				if (!texture.second->m_TransitionTransferDstToShaderReadOnly)
				{
					texture.second->TransitionSubResources(textureTransferDstToShaderReadOnlyBarrier,
						{ { Barrier::AccessBit::TRANSFER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT,
						Image::Layout::TRANSFER_DST_OPTIMAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
					texture.second->m_TransitionTransferDstToShaderReadOnly = true;
				}
			}
		}
	}

	bool preTransferGraphicsCmdBuffer = textureShaderReadOnlyBarrierToTransferDst.size();
	bool transferCmdBuffer = forceUploadCamera || forceUploadLights || forceUploadMeshes || forceUploadSkybox || textureUnknownToTransferDstBarrier.size();
	bool asyncComputeCmdBufferMipMaps = generateMipmaps || !m_Skybox->m_Generated;
	bool postTransferGraphicsCmdBuffer = textureTransferDstToShaderReadOnlyBarrier.size();

	//Upload Pre-Transfer Graphics CmdBuffer
	{
		if (preTransferGraphicsCmdBuffer)
		{
			m_CmdBuffer->Reset(3, false);
			m_CmdBuffer->Begin(3, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

			if (textureShaderReadOnlyBarrierToTransferDst.size())
				m_CmdBuffer->PipelineBarrier(3, PipelineStageBit::FRAGMENT_SHADER_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, textureShaderReadOnlyBarrierToTransferDst);

			m_CmdBuffer->End(3);
			m_CmdBuffer->Submit({ 3 }, {}, {}, { transfer0 }, preTransferGraphicsFence);
		}
	}

	//Upload Transfer CmdBuffer
	{
		if (transferCmdBuffer)
		{
			m_TransCmdBuffer->Reset(0, false);
			m_TransCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
			if (m_Camera)
				m_Camera->GetUB()->Upload(m_TransCmdBuffer, 0, forceUploadCamera);
			if (m_Lights[0])
				m_Lights[0]->GetUB()->Upload(m_TransCmdBuffer, 0, forceUploadLights);
			if (m_Skybox)
				m_Skybox->GetUB()->Upload(m_TransCmdBuffer, 0, forceUploadSkybox);

			for (auto& model : m_RenderQueue)
			{
				for (auto& vb : model->GetMesh()->GetVertexBuffers())
					vb->Upload(m_TransCmdBuffer, 0, forceUploadMeshes);
				for (auto& ib : model->GetMesh()->GetIndexBuffers())
					ib->Upload(m_TransCmdBuffer, 0, forceUploadMeshes);

				model->GetUB()->Upload(m_TransCmdBuffer, 0);
				model->GetMesh()->GetMaterials()[0]->GetUB()->Upload(m_TransCmdBuffer, 0);
			}

			if (textureUnknownToTransferDstBarrier.size())
				m_TransCmdBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, textureUnknownToTransferDstBarrier);

			for (auto& model : m_RenderQueue)
			{
				for (auto& material : model->GetMesh()->GetMaterials())
				{
					for (auto& texture : material->GetTextures())
					{
						texture.second->Upload(m_TransCmdBuffer, 0);
					}
				}
			}
			m_TransCmdBuffer->End(0);

			if (preTransferGraphicsCmdBuffer)
				m_TransCmdBuffer->Submit({ 0 }, { transfer0 }, { PipelineStageBit::TRANSFER_BIT }, { transfer1 }, transferFence);
			else
				m_TransCmdBuffer->Submit({ 0 }, {}, {}, { transfer1 }, transferFence);
		}
	}

	if (asyncComputeCmdBufferMipMaps && transferCmdBuffer)
		transferFence->Wait();

	//Generate MipMaps on independent Compute Queue
	{
		if (asyncComputeCmdBufferMipMaps)
		{
			std::vector<std::future<void>> mipmapsDispatchs;
			for (auto& model : m_RenderQueue)
			{
				for (auto& material : model->GetMesh()->GetMaterials())
				{
					for (auto& texture : material->GetTextures())
					{
						ImageProcessing::GenerateMipMaps(
							{
								texture.second,
								Barrier::AccessBit::TRANSFER_WRITE_BIT,
								Barrier::AccessBit::TRANSFER_READ_BIT,
								Image::Layout::TRANSFER_DST_OPTIMAL,
								Image::Layout::TRANSFER_DST_OPTIMAL,
							}
						);
						//auto generateMipMaps_texture = [](gear::Ref<Texture> texture) { texture->GenerateMipMaps(); };
						//mipmapsDispatchs.push_back(std::async(std::launch::async, generateMipMaps_texture, texture.second));
					}
				}
			}
			for (auto& dispatch : mipmapsDispatchs)
			{
				//dispatch.wait();
			}

			if (!m_Skybox->m_Cubemap && !m_Skybox->m_Generated)
			{
				ImageProcessing::EquirectangularToCube(
					{
						m_Skybox->GetGeneratedCubemap(),
						Barrier::AccessBit::SHADER_WRITE_BIT,
						Barrier::AccessBit::SHADER_READ_BIT,
						Image::Layout::UNKNOWN,
						Image::Layout::SHADER_READ_ONLY_OPTIMAL,
					},
					{
						m_Skybox->GetTexture(),
						Barrier::AccessBit::TRANSFER_WRITE_BIT,
						Barrier::AccessBit::TRANSFER_READ_BIT,
						Image::Layout::TRANSFER_DST_OPTIMAL,
						Image::Layout::TRANSFER_DST_OPTIMAL,
					}
				);
				m_Skybox->m_Generated = true;
			}
		}
	}

	//Upload Post-Transfer Graphics CmdBuffer
	{
		if (postTransferGraphicsCmdBuffer)
		{
			m_CmdBuffer->Reset(2, false);
			m_CmdBuffer->Begin(2, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

			m_CmdBuffer->PipelineBarrier(2, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, DependencyBit::NONE_BIT, textureTransferDstToShaderReadOnlyBarrier);

			m_CmdBuffer->End(2);

			if (transferCmdBuffer)
				m_CmdBuffer->Submit({ 2 }, { transfer1 }, { PipelineStageBit::TRANSFER_BIT }, {}, postTransferGraphicsFence);
			else
				m_CmdBuffer->Submit({ 2 }, {}, {}, {}, postTransferGraphicsFence);
		}
	}

	if (preTransferGraphicsCmdBuffer)
		preTransferGraphicsFence->Wait();
	if (!asyncComputeCmdBufferMipMaps && transferCmdBuffer)
		transferFence->Wait();
	if (postTransferGraphicsCmdBuffer)
		postTransferGraphicsFence->Wait();

	m_ReloadTextures = false;
}

void Renderer::Flush()
{
	if(!m_BuiltDescPoolsAndSets)
	{
		//Desriptor Pool
		std::map<DescriptorType, uint32_t> poolSizesMap;
		size_t m_RenderQueueMaterialCount = 0;
		for (auto& model : m_RenderQueue)
		{
			const miru::Ref<Pipeline>& pipeline = m_RenderPipelines[model->GetPipelineName()]->GetPipeline();
			const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = m_RenderPipelines[model->GetPipelineName()]->GetRBDs();
			size_t materialCount = model->GetMesh()->GetMaterials().size();
			m_RenderQueueMaterialCount += materialCount;

			uint32_t set = 0;
			uint32_t binding = 0;
			for (auto& set_rbds : rbds)
			{
				for (auto& binding_rbds : set_rbds)
				{
					uint32_t& descCount = poolSizesMap[binding_rbds.type];
					descCount += (binding_rbds.descriptorCount * (set == 2 ? static_cast<uint32_t>(materialCount) : 1));
					binding++;
				}
				set++;
			}
		}
		m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_Renderer";
		m_DescPoolCI.device = m_Device;
		for (auto& poolSize : poolSizesMap)
			m_DescPoolCI.poolSizes.push_back({ poolSize.first, poolSize.second });
		m_DescPoolCI.maxSets = static_cast<uint32_t>(m_RenderPipelines.size() + m_RenderQueue.size() + m_RenderQueueMaterialCount);
		m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

		//Per view Descriptor Set
		for (auto& pipeline : m_RenderPipelines)
		{
			const std::vector<miru::Ref<DescriptorSetLayout>>& descriptorSetLayouts = pipeline.second->GetDescriptorSetLayouts();
			const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = pipeline.second->GetRBDs();

			DescriptorSet::CreateInfo descSetPerViewCI;
			descSetPerViewCI.debugName = "GEAR_CORE_DescriptorSet_PerView: " + pipeline.second->GetPipeline()->GetCreateInfo().debugName;
			descSetPerViewCI.pDescriptorPool = m_DescPool;
			descSetPerViewCI.pDescriptorSetLayouts = { descriptorSetLayouts[0] };
			m_DescSetPerView[pipeline.second] = DescriptorSet::Create(&descSetPerViewCI);
			
			for (auto& rbd : rbds[0])
			{
				const uint32_t& binding = rbd.binding;
				const std::string& name = core::toupper(rbd.name);
				if (rbd.structSize > 0)
				{
					if (SetUpdateTypeMap.find(name) == SetUpdateTypeMap.end())
						continue;
					if (SetUpdateTypeMap[name] != SetUpdateType::PER_VIEW)
						continue;
				}

				if (name.compare("CAMERA") == 0)
				{
					m_DescSetPerView[pipeline.second]->AddBuffer(0, binding, { { m_Camera->GetUB()->GetBufferView() } });
				}
				else if (name.compare("LIGHTS") == 0)
				{
					m_DescSetPerView[pipeline.second]->AddBuffer(0, binding, { { m_Lights[0]->GetUB()->GetBufferView() } });
				}
				
				else
					continue;
			}
			m_DescSetPerView[pipeline.second]->Update();
		}

		//Per model Descriptor Sets
		for (auto& model : m_RenderQueue)
		{
			const std::vector<miru::Ref<DescriptorSetLayout>>& descriptorSetLayouts = m_RenderPipelines[model->GetPipelineName()]->GetDescriptorSetLayouts();
			const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = m_RenderPipelines[model->GetPipelineName()]->GetRBDs();

			DescriptorSet::CreateInfo descSetPerModelCI;
			descSetPerModelCI.debugName = "GEAR_CORE_DescriptorSet_PerModel: " + model->GetDebugName();
			descSetPerModelCI.pDescriptorPool = m_DescPool;
			descSetPerModelCI.pDescriptorSetLayouts = { descriptorSetLayouts[1] };
			m_DescSetPerModel[model] = DescriptorSet::Create(&descSetPerModelCI);

			for (auto& rbd : rbds[1])
			{
				const uint32_t& binding = rbd.binding;
				const std::string& name = core::toupper(rbd.name);
				if (rbd.structSize > 0)
				{
					if (SetUpdateTypeMap.find(name) == SetUpdateTypeMap.end())
						continue;
					if (SetUpdateTypeMap[name] != SetUpdateType::PER_MODEL)
						continue;

					if (name.compare("MODEL") == 0)
					{
						m_DescSetPerModel[model]->AddBuffer(0, binding, { { model->GetUB()->GetBufferView() } });
					}
					else
						continue;
				}
			}
			m_DescSetPerModel[model]->Update();
		}

		//Per material Descriptor Sets
		for (auto& model : m_RenderQueue)
		{
			const std::vector<miru::Ref<DescriptorSetLayout>>& descriptorSetLayouts = m_RenderPipelines[model->GetPipelineName()]->GetDescriptorSetLayouts();
			const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = m_RenderPipelines[model->GetPipelineName()]->GetRBDs();
			
			for (auto& material : model->GetMesh()->GetMaterials())
			{
				DescriptorSet::CreateInfo descSetPerMaterialCI;
				descSetPerMaterialCI.debugName = "GEAR_CORE_DescriptorSet_PerMaterial: " + material->GetDebugName();
				descSetPerMaterialCI.pDescriptorPool = m_DescPool;
				descSetPerMaterialCI.pDescriptorSetLayouts = { descriptorSetLayouts[2] };
				m_DescSetPerMaterial[material] = DescriptorSet::Create(&descSetPerMaterialCI);

				for (auto& rbd : rbds[2])
				{
					const uint32_t& binding = rbd.binding;
					const std::string& name = core::toupper(rbd.name);
					if (rbd.structSize > 0)
					{
						if (SetUpdateTypeMap.find(name) == SetUpdateTypeMap.end())
							continue;
						if (SetUpdateTypeMap[name] != SetUpdateType::PER_MATERIAL)
							continue;
					}
					
					if (name.compare("SKYBOXINFO") == 0)
					{
						const gear::Ref<Material>& material = m_Skybox->GetModel()->GetMesh()->GetMaterials()[0];
						m_DescSetPerMaterial[material]->AddBuffer(0, binding, { { m_Skybox->GetUB()->GetBufferView() } });
					}
					else if (name.find("SKYBOX") == 0)
					{
						const gear::Ref<Material>& material = m_Skybox->GetModel()->GetMesh()->GetMaterials()[0];
						const gear::Ref<Texture>& skyboxTexture = m_Skybox->GetGeneratedCubemap(); //material->GetTextures()[Material::TextureType::ALBEDO];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { { skyboxTexture->GetTextureSampler(), skyboxTexture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
					}

					else if (name.compare("PBRCONSTANTS") == 0)
					{
						m_DescSetPerMaterial[material]->AddBuffer(0, binding, { { material->GetUB()->GetBufferView() } });
					}
					else if (name.find("NORMAL") == 0)
					{
						const gear::Ref<Texture>& texture = material->GetTextures()[Material::TextureType::NORMAL];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("ALBEDO") == 0)
					{
						const gear::Ref<Texture>& texture = model->GetMesh()->GetMaterials()[0]->GetTextures()[Material::TextureType::ALBEDO];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("METALLIC") == 0)
					{
						const gear::Ref<Texture>& texture = model->GetMesh()->GetMaterials()[0]->GetTextures()[Material::TextureType::METALLIC];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("ROUGHNESS") == 0)
					{
						const gear::Ref<Texture>& texture = model->GetMesh()->GetMaterials()[0]->GetTextures()[Material::TextureType::ROUGHNESS];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("AMBIENTOCCLUSION") == 0)
					{
						const gear::Ref<Texture>& texture = model->GetMesh()->GetMaterials()[0]->GetTextures()[Material::TextureType::AMBIENT_OCCLUSION];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("EMISSIVE") == 0)
					{
						const gear::Ref<Texture>& texture = model->GetMesh()->GetMaterials()[0]->GetTextures()[Material::TextureType::EMISSIVE];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else
						continue;
				
				}
				m_DescSetPerMaterial[material]->Update();
			}
		}

		m_BuiltDescPoolsAndSets = true;
	}

	//Record Present CmdBuffers
	m_DrawFences[m_FrameIndex]->Wait();
	{
		m_CmdBuffer->Reset(m_FrameIndex, false);
		m_CmdBuffer->Begin(m_FrameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
		m_CmdBuffer->BeginRenderPass(m_FrameIndex, m_Framebuffers[m_FrameIndex], { {0.25f, 0.25f, 0.25f, 1.0f}, {1.0f, 0} });

		for (auto& model : m_RenderQueue)
		{
			const gear::Ref<graphics::RenderPipeline>& renderPipeline = m_RenderPipelines[model->GetPipelineName()];
			const miru::Ref<Pipeline>& pipeline = renderPipeline->GetPipeline();

			m_CmdBuffer->BindPipeline(m_FrameIndex, pipeline);

			for (size_t i = 0; i < model->GetMesh()->GetVertexBuffers().size(); i++)
			{
				gear::Ref<objects::Material> material = model->GetMesh()->GetMaterials()[i];
				m_CmdBuffer->BindDescriptorSets(m_FrameIndex, { m_DescSetPerView[renderPipeline], m_DescSetPerModel[model], m_DescSetPerMaterial[material] }, pipeline);
				
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

void Renderer::ResizeRenderPipelineViewports(uint32_t width, uint32_t height)
{
	m_Context->DeviceWaitIdle();
	for (auto& renderPipeline : m_RenderPipelines)
	{
		renderPipeline.second->m_CI.viewportState.viewports = { { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f } };
		renderPipeline.second->m_CI.viewportState.scissors = { { { 0, 0 },{ width, height } } };
		renderPipeline.second->Rebuild();
	}
}

void Renderer::RecompileRenderPipelineShaders()
{
	m_Context->DeviceWaitIdle();
	for (auto& renderPipeline : m_RenderPipelines)
		renderPipeline.second->RecompileShaders();
}

void Renderer::ReloadTextures()
{
	m_Context->DeviceWaitIdle();
	m_ReloadTextures = true;
}
