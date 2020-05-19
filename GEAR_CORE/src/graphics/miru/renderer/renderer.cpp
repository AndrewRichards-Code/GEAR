#include "renderer.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OBJECTS;

using namespace miru;
using namespace miru::crossplatform;

Renderer::Renderer(miru::Ref<miru::crossplatform::Context> context)
{
	m_CmdPoolCI.debugName = "GEAR_CORE_CommandPool_Renderer";
	m_CmdPoolCI.pContext = context;
	m_CmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_CmdPoolCI.queueFamilyIndex = 0;
	m_CmdPool = CommandPool::Create(&m_CmdPoolCI);

	m_CmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_Renderer";;
	m_CmdBufferCI.pCommandPool = m_CmdPool;
	m_CmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_CmdBufferCI.commandBufferCount = 3;
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
	m_TransCmdBuffer = CommandBuffer::Create(&m_TransCmdBufferCI);
}

Renderer::~Renderer()
{
}

void Renderer::Submit(Object* obj)
{
	m_RenderQueue.push_back((Object*) obj);
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
		m_Camera->GetUBO()->Upload(m_TransCmdBuffer, 0);
		m_Lights[0]->GetUBO0()->Upload(m_TransCmdBuffer, 0);
		m_Lights[0]->GetUBO1()->Upload(m_TransCmdBuffer, 0);

		std::vector<Ref<Barrier>> initialBarrier;
		for (auto& obj : m_RenderQueue)
		{
			for (auto& vb : obj->GetVBOs())
				vb->Upload(m_TransCmdBuffer, 0);
			obj->GetIBO()->Upload(m_TransCmdBuffer, 0);
			obj->GetUBO()->Upload(m_TransCmdBuffer, 0);

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

	while (transferFence->Wait()) {	}

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
	descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::FRAGMENT_BIT} };
	m_DescSetLayouts.push_back(DescriptorSetLayout::Create(&descSetLayoutCI));
	descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::FRAGMENT_BIT} };
	m_DescSetLayouts.push_back(DescriptorSetLayout::Create(&descSetLayoutCI));

	m_DescSetCI.debugName = "GEAR_CORE_DescSetRenderer";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = { m_DescSetLayouts[0] };
	m_DescSetCamera = DescriptorSet::Create(&m_DescSetCI);
	m_DescSetCamera->AddBuffer(0, 0, { { m_Camera->GetUBO()->GetBufferView() } });
	m_DescSetCamera->Update();

	m_DescSetCI.pDescriptorSetLayouts = { m_DescSetLayouts[2], m_DescSetLayouts[3] };
	m_DescSetLight = DescriptorSet::Create(&m_DescSetCI);
	m_DescSetLight->AddBuffer(0, 0, { { m_Lights[0]->GetUBO0()->GetBufferView() } });
	m_DescSetLight->AddBuffer(1, 0, { { m_Lights[0]->GetUBO1()->GetBufferView() } });
	m_DescSetLight->Update();

	for (auto& obj : m_RenderQueue)
	{
		m_DescSetCI.pDescriptorSetLayouts = { m_DescSetLayouts[1] };
		m_DescSetObj[obj] = DescriptorSet::Create(&m_DescSetCI);
		m_DescSetObj[obj]->AddImage(0, 1, { {obj->GetTexture()->GetTextureSampler(), obj->GetTexture()->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
		m_DescSetObj[obj]->AddBuffer(0, 0, { { obj->GetUBO()->GetBufferView() } });
		m_DescSetObj[obj]->Update();
	}

	//Record Present CmdBuffers
	for (uint32_t i = 0; i < 2; i++)
	{
		m_CmdBuffer->Begin(i, CommandBuffer::UsageBit::SIMULTANEOUS);
		m_CmdBuffer->BeginRenderPass(i, m_Framebuffers[i], { {0.25f, 0.25f, 0.25f, 1.0f}, {1.0f, 0} });
		
		for(auto& obj : m_RenderQueue)
		{
			m_CmdBuffer->BindPipeline(i, obj->GetPipeline()->GetPipeline());

			m_CmdBuffer->BindDescriptorSets(i, { m_DescSetCamera, m_DescSetObj[obj], m_DescSetLight }, obj->GetPipeline()->GetPipeline());
			
			std::vector<Ref<BufferView>> vbv;
			for (auto& vb : obj->GetVBOs())
				vbv.push_back(vb->GetVertexBufferView());
			m_CmdBuffer->BindVertexBuffers(i, vbv);
			m_CmdBuffer->BindIndexBuffer(i, obj->GetIBO()->GetIndexBufferView());

			m_CmdBuffer->DrawIndexed(i, obj->GetIBO()->GetCount());
		}
		m_CmdBuffer->EndRenderPass(i);
		m_CmdBuffer->End(i);
	}
	m_RenderQueue.clear();
}

void Renderer::UpdateCamera()
{
	Fence::CreateInfo transFenceCI = { "GEAR_CORE_FenceRenderTransfer", m_TransCmdPoolCI.pContext->GetDevice(), false, UINT64_MAX };
	Ref<Fence> transferFence = Fence::Create(&transFenceCI);
	{
		m_TransCmdBuffer->Reset(0, false);
		m_TransCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
		m_Camera->GetUBO()->Upload(m_TransCmdBuffer, 0, true);
		m_TransCmdBuffer->End(0);
	}
	m_TransCmdBuffer->Submit({ 0 }, {}, {}, PipelineStageBit::TRANSFER_BIT, transferFence);
	while (transferFence->Wait()) {}
}