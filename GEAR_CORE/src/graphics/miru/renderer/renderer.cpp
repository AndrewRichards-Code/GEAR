#include "renderer.h"

using namespace gear;
using namespace graphics;
using namespace objects;

using namespace miru;
using namespace miru::crossplatform;

Renderer::Renderer(miru::Ref<miru::crossplatform::Context> context)
{
	//Renderer and Transfer CmdPools and CmdBuffers
	m_CmdPoolCI.debugName = "GEAR_CORE_CommandPool_Renderer";
	m_CmdPoolCI.pContext = context;
	m_CmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_CmdPoolCI.queueFamilyIndex = 0;
	m_CmdPool = CommandPool::Create(&m_CmdPoolCI);

	m_CmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_Renderer";;
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

	m_TransCmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_Renderer_Transfer";;
	m_TransCmdBufferCI.pCommandPool = m_TransCmdPool;
	m_TransCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_TransCmdBufferCI.commandBufferCount = 1;
	m_TransCmdBufferCI.allocateNewCommandPoolPerBuffer = false;
	m_TransCmdBuffer = CommandBuffer::Create(&m_TransCmdBufferCI);

	//Present Synchronisation
	m_DrawFenceCI.debugName = "GEAR_CORE_Sync_DrawFence";
	m_DrawFenceCI.device = context->GetDevice();
	m_DrawFenceCI.signaled = true;
	m_DrawFenceCI.timeout = UINT64_MAX;
	m_DrawFences = { Fence::Create(&m_DrawFenceCI), Fence::Create(&m_DrawFenceCI) };

	m_AcquireSemaphoreCI.debugName = "GEAR_CORE_Sync_AcquireSeamphore";
	m_AcquireSemaphoreCI.device = context->GetDevice();
	m_AcquireSemaphores = { Semaphore::Create(&m_AcquireSemaphoreCI), Semaphore::Create(&m_AcquireSemaphoreCI) };

	m_SubmitSemaphoreCI.debugName = "GEAR_CORE_Sync_AcquireSeamphore";
	m_SubmitSemaphoreCI.device = context->GetDevice();
	m_SubmitSemaphores = { Semaphore::Create(&m_SubmitSemaphoreCI), Semaphore::Create(&m_SubmitSemaphoreCI) };
}

Renderer::~Renderer()
{
}

void Renderer::Submit(gear::Ref<Model> obj)
{
	m_RenderQueue.push_back(obj);
}

void Renderer::Flush()
{
	//Upload CmdBufer
	Semaphore::CreateInfo transSemaphoreCI = { "GEAR_CORE_SemaphoreRenderTransfer", m_TransCmdPoolCI.pContext->GetDevice() };
	Ref<Semaphore> transfer = Semaphore::Create(&transSemaphoreCI);
	Fence::CreateInfo transFenceCI = { "GEAR_CORE_FenceRenderTransfer", m_TransCmdPoolCI.pContext->GetDevice(), false, UINT64_MAX };
	Ref<Fence> transferFence = Fence::Create(&transFenceCI);
	{
		m_TransCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
		m_Camera->GetUB()->Upload(m_TransCmdBuffer, 0);
		m_Lights[0]->GetUB0()->Upload(m_TransCmdBuffer, 0);
		m_Lights[0]->GetUB1()->Upload(m_TransCmdBuffer, 0);

		std::vector<Ref<Barrier>> initialBarrier;
		for (auto& obj : m_RenderQueue)
		{
			for (auto& vb : obj->GetVBs())
				vb.second->Upload(m_TransCmdBuffer, 0);
			obj->GetIB()->Upload(m_TransCmdBuffer, 0);
			obj->GetUB()->Upload(m_TransCmdBuffer, 0);

			obj->GetTexture()->GetInitialTransition(initialBarrier);
		}

		m_TransCmdBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, initialBarrier);

		for (auto& obj : m_RenderQueue)
		{
			obj->GetTexture()->Upload(m_TransCmdBuffer, 0);
		}

		m_TransCmdBuffer->End(0);
	}
	m_TransCmdBuffer->Submit({ 0 }, {}, { transfer }, PipelineStageBit::TRANSFER_BIT, nullptr);
	{
		m_CmdBuffer->Begin(2, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
		std::vector<Ref<Barrier>> finalBarrier;
		for (auto& obj : m_RenderQueue)
		{
			obj->GetTexture()->GetFinalTransition(finalBarrier);
		}

		m_CmdBuffer->PipelineBarrier(2, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, DependencyBit::NONE_BIT, finalBarrier);
		m_CmdBuffer->End(2);
	}
	m_CmdBuffer->Submit({ 2 }, { transfer }, {}, PipelineStageBit::TRANSFER_BIT, transferFence);

	transferFence->Wait();

	//Build DescriptorPools and Sets
	m_DescPoolCI.debugName = "GEAR_CORE_DescPoolRenderer";
	m_DescPoolCI.device = m_TransCmdPoolCI.pContext->GetDevice();
	m_DescPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, (uint32_t)m_RenderQueue.size()}, {DescriptorType::UNIFORM_BUFFER, (uint32_t)m_RenderQueue.size() + 3} };
	m_DescPoolCI.maxSets = (uint32_t)m_RenderQueue.size() + 3;
	m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	DescriptorSetLayout::CreateInfo descSetLayoutCI;
	descSetLayoutCI.debugName = "GEAR_CORE_DescSetLayoutRenderer";
	descSetLayoutCI.device = m_TransCmdPoolCI.pContext->GetDevice();
	descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT} };
	m_DescSetLayouts.push_back(DescriptorSetLayout::Create(&descSetLayoutCI));
	descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT}, {1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT} };
	m_DescSetLayouts.push_back(DescriptorSetLayout::Create(&descSetLayoutCI));
	descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::FRAGMENT_BIT}, {1, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::FRAGMENT_BIT} };
	m_DescSetLayouts.push_back(DescriptorSetLayout::Create(&descSetLayoutCI));

	m_DescSetCI.debugName = "GEAR_CORE_DescSetRenderer";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = { m_DescSetLayouts[0] };
	m_DescSetCamera = DescriptorSet::Create(&m_DescSetCI);
	m_DescSetCamera->AddBuffer(0, 0, { { m_Camera->GetUB()->GetBufferView() } });
	m_DescSetCamera->Update();

	m_DescSetCI.pDescriptorSetLayouts = { m_DescSetLayouts[2] };
	m_DescSetLight = DescriptorSet::Create(&m_DescSetCI);
	m_DescSetLight->AddBuffer(0, 0, { { m_Lights[0]->GetUB0()->GetBufferView() } });
	m_DescSetLight->AddBuffer(0, 1, { { m_Lights[0]->GetUB1()->GetBufferView() } });
	m_DescSetLight->Update();

	for (auto& obj : m_RenderQueue)
	{
		m_DescSetCI.pDescriptorSetLayouts = { m_DescSetLayouts[1] };
		m_DescSetObj[obj] = DescriptorSet::Create(&m_DescSetCI);
		m_DescSetObj[obj]->AddImage(0, 1, { {obj->GetTexture()->GetTextureSampler(), obj->GetTexture()->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
		m_DescSetObj[obj]->AddBuffer(0, 0, { { obj->GetUB()->GetBufferView() } });
		m_DescSetObj[obj]->Update();
	}

	//Record Present CmdBuffers
	{
		m_CmdBuffer->Reset(m_FrameIndex, false);
		m_CmdBuffer->Begin(m_FrameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
		m_CmdBuffer->BeginRenderPass(m_FrameIndex, m_Framebuffers[m_FrameIndex], { {0.25f, 0.25f, 0.25f, 1.0f}, {1.0f, 0} });

		for (auto& obj : m_RenderQueue)
		{
			m_CmdBuffer->BindPipeline(m_FrameIndex, obj->GetPipeline()->GetPipeline());

			std::vector<Ref<DescriptorSet>> bindDescriptorSets;
			for (size_t descCount = 0; descCount < obj->GetPipeline()->GetDescriptorSetLayouts().size(); descCount++)
			{
				switch (descCount)
				{
				case 0:
					bindDescriptorSets.push_back(m_DescSetCamera); continue;
				case 1:
					bindDescriptorSets.push_back(m_DescSetObj[obj]); continue;
				case 2:
					bindDescriptorSets.push_back(m_DescSetLight); continue;
				default:
					continue;
				}
			}
			m_CmdBuffer->BindDescriptorSets(m_FrameIndex, bindDescriptorSets, obj->GetPipeline()->GetPipeline());

			std::vector<Ref<BufferView>> vbv;
			for (auto& vb : obj->GetVBs())
				vbv.push_back(vb.second->GetVertexBufferView());
			m_CmdBuffer->BindVertexBuffers(m_FrameIndex, vbv);
			m_CmdBuffer->BindIndexBuffer(m_FrameIndex, obj->GetIB()->GetIndexBufferView());

			m_CmdBuffer->DrawIndexed(m_FrameIndex, obj->GetIB()->GetCount());
		}
		m_CmdBuffer->EndRenderPass(m_FrameIndex);
		m_CmdBuffer->End(m_FrameIndex);
	}

	m_RenderQueue.clear();
}

void Renderer::Present(const miru::Ref<miru::crossplatform::Swapchain>& swapchain, bool& windowResize)
{
	m_CmdBuffer->Present({ 0, 1 }, swapchain, m_DrawFences, m_AcquireSemaphores, m_SubmitSemaphores, windowResize);
	m_FrameIndex = (m_FrameIndex + 1) % swapchain->GetCreateInfo().swapchainCount;
	m_FrameCount++;
}

void Renderer::UpdateCamera()
{
	Fence::CreateInfo transFenceCI = { "GEAR_CORE_FenceRenderTransfer", m_TransCmdPoolCI.pContext->GetDevice(), false, UINT64_MAX };
	Ref<Fence> transferFence = Fence::Create(&transFenceCI);
	{
		m_TransCmdBuffer->Reset(0, false);
		m_TransCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
		m_Camera->GetUB()->Upload(m_TransCmdBuffer, 0, true);
		m_TransCmdBuffer->End(0);
	}
	m_TransCmdBuffer->Submit({ 0 }, {}, {}, PipelineStageBit::TRANSFER_BIT, transferFence);
	transferFence->Wait();
}