#include "gear_core_common.h"
#include "Renderer.h"

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
}

void Renderer::InitialiseRenderPipelines(const std::vector<std::string>& filepaths, float viewportWidth, float viewportHeight, const miru::Ref<RenderPass>& renderPass)
{
	RenderPipeline::LoadInfo renderPipelineLI;
	for (auto& filepath : filepaths)
	{
		//TODO: What if the Pipeline is already loaded?
		renderPipelineLI.device = m_Device;
		renderPipelineLI.filepath = filepath;
		renderPipelineLI.viewportWidth = viewportWidth;
		renderPipelineLI.viewportHeight = viewportHeight;
		renderPipelineLI.renderPass = renderPass;
		renderPipelineLI.subpassIndex = 0;
		Ref<RenderPipeline> renderPipeline = CreateRef<RenderPipeline>(&renderPipelineLI);
		m_RenderPipelines[renderPipeline->m_CI.debugName] = renderPipeline;
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
					texture.second->GetTransition_Initial(initialBarrier);
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
					texture.second->GetTransition_ToShaderReadOnly(finalBarrier);
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
			modelMaterialSetCI.debugName = "GEAR_CORE_DescriptorSet_ModelMaterial: " + model->GetDebugName();
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