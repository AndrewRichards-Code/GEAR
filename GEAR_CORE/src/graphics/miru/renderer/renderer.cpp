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
	m_TransCmdPoolCI.queueFamilyIndex = 0;
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
	Semaphore::CreateInfo transSemaphoreCI = { "GEAR_CORE_SemaphoreRenderTransfer", m_TransCmdPoolCI.pContext->GetDevice() };
	Ref<Semaphore> transfer = Semaphore::Create(&transSemaphoreCI);
	{

		m_TransCmdBuffer->Begin(0, CommandBuffer::UsageBit::SIMULTANEOUS);
		m_Camera->GetUBO()->Upload(m_TransCmdBuffer, 0);

		std::vector<Ref<Barrier>> initialBarrier;
		for (auto& obj : m_RenderQueue)
		{
			for (auto& vb : obj->GetVBOs())
				vb->Upload(m_TransCmdBuffer, 0);
			obj->GetIBO()->Upload(m_TransCmdBuffer, 0);
			obj->GetUBO()->Upload(m_TransCmdBuffer, 0);

			obj->GetTexture()->GetInitialTransition(initialBarrier);
		}

		m_TransCmdBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, initialBarrier);

		for (auto& obj : m_RenderQueue)
		{
			obj->GetTexture()->Upload(m_TransCmdBuffer, 0);
		}

		m_TransCmdBuffer->End(0);
	}
	m_TransCmdBuffer->Submit({ 0 }, {}, { transfer }, PipelineStageBit::TRANSFER_BIT, nullptr);

	{
		std::vector<Ref<Barrier>> finalBarrier;
		for (auto& obj : m_RenderQueue)
		{
			obj->GetTexture()->GetFinalTransition(finalBarrier);
		}

		m_CmdBuffer->PipelineBarrier(2, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, finalBarrier);
	}
	m_CmdBuffer->Submit({ 2 }, { transfer }, {}, PipelineStageBit::TRANSFER_BIT, nullptr);

	m_DescPoolCI.debugName = "GEAR_CORE_DescPoolRenderer";
	m_DescPoolCI.device = m_TransCmdPoolCI.pContext->GetDevice();
	m_DescPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, (uint32_t)m_RenderQueue.size()}, {DescriptorType::UNIFORM_BUFFER, (uint32_t)m_RenderQueue.size() + 1} };
	m_DescPoolCI.maxSets = (uint32_t)m_RenderQueue.size() + 1;
	m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

	DescriptorSetLayout::CreateInfo descSetLayoutCI;
	descSetLayoutCI.debugName = "GEAR_CORE_DescSetLayoutRenderer";
	descSetLayoutCI.device = m_TransCmdPoolCI.pContext->GetDevice();
	descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT}, {1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT} };
	Ref<DescriptorSetLayout> descSetLayout1 = DescriptorSetLayout::Create(&descSetLayoutCI);
	descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT} };
	Ref<DescriptorSetLayout> descSetLayout0 = DescriptorSetLayout::Create(&descSetLayoutCI);

	m_DescSetCI.debugName = "GEAR_CORE_DescSetRenderer";
	m_DescSetCI.pDescriptorPool = m_DescPool;
	m_DescSetCI.pDescriptorSetLayouts = { descSetLayout0 };
	m_DescSetCamera = DescriptorSet::Create(&m_DescSetCI);
	m_DescSetCamera->AddBuffer(0, 0, { { m_Camera->GetUBO()->GetBufferView() } });
	m_DescSetCamera->Update();

	for (uint32_t i = 0; i < 2; i++)
	{
		m_CmdBuffer->Begin(i, CommandBuffer::UsageBit::SIMULTANEOUS);
		m_CmdBuffer->BeginRenderPass(i, m_Framebuffers[i], { {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0} });
		while (!m_RenderQueue.empty())
		{
			Object* obj = m_RenderQueue.front();
			m_CmdBuffer->BindPipeline(i, obj->GetPipeline()->GetPipeline());
			
			std::vector<Ref<BufferView>> vbv;
			for (auto& vb : obj->GetVBOs())
				vbv.push_back(vb->GetVertexBufferView());
			m_CmdBuffer->BindVertexBuffers(i, vbv);
			m_CmdBuffer->BindIndexBuffer(i, obj->GetIBO()->GetIndexBufferView());

			m_DescSetCI.pDescriptorSetLayouts = { descSetLayout1 };
			m_DescSetObj.emplace_back(DescriptorSet::Create(&m_DescSetCI).get());
			m_DescSetObj.back()->AddImage(1, 1, { {obj->GetTexture()->GetTextureSampler(), obj->GetTexture()->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			m_DescSetObj.back()->AddBuffer(1, 0, { { obj->GetUBO()->GetBufferView() } });
			m_DescSetObj.back()->Update();
			m_CmdBuffer->BindDescriptorSets(i, { m_DescSetCamera, m_DescSetObj.back() }, obj->GetPipeline()->GetPipeline());

			m_CmdBuffer->DrawIndexed(i, obj->GetIBO()->GetCount());

			m_RenderQueue.pop_front();
		}
		m_CmdBuffer->EndRenderPass(i);
		m_CmdBuffer->End(i);
	}
}