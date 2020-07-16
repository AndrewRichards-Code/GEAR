#include "gear_core_common.h"
#include "renderer.h"

using namespace gear;
using namespace graphics;
using namespace objects;

using namespace miru;
using namespace miru::crossplatform;

Renderer::Renderer(const miru::Ref<miru::crossplatform::Context>& context)
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
	m_TransCmdBufferCI.allocateNewCommandPoolPerBuffer = GraphicsAPI::IsD3D12();
	m_TransCmdBuffer = CommandBuffer::Create(&m_TransCmdBufferCI);

	m_Context = context;
	m_Device = context->GetDevice();
	
	//Present Synchronisation
	m_DrawFenceCI.debugName = "GEAR_CORE_Sync_DrawFence";
	m_DrawFenceCI.device = m_Device;
	m_DrawFenceCI.signaled = true;
	m_DrawFenceCI.timeout = UINT64_MAX;
	m_DrawFences = { Fence::Create(&m_DrawFenceCI), Fence::Create(&m_DrawFenceCI) };

	m_AcquireSemaphoreCI.debugName = "GEAR_CORE_Sync_AcquireSeamphore";
	m_AcquireSemaphoreCI.device = m_Device;
	m_AcquireSemaphores = { Semaphore::Create(&m_AcquireSemaphoreCI), Semaphore::Create(&m_AcquireSemaphoreCI) };

	m_SubmitSemaphoreCI.debugName = "GEAR_CORE_Sync_AcquireSeamphore";
	m_SubmitSemaphoreCI.device = m_Device;
	m_SubmitSemaphores = { Semaphore::Create(&m_SubmitSemaphoreCI), Semaphore::Create(&m_SubmitSemaphoreCI) };

}

Renderer::~Renderer()
{
	m_Context->DeviceWaitIdle();
	ClearupRenderPipelines();
}

void Renderer::InitialiseRenderPipelines(float viewportWidth, float viewportHeight, const miru::Ref<miru::crossplatform::RenderPass>& renderPass)
{
	std::string mscDirectory = "../GEAR_CORE/dep/MIRU/MIRU_SHADER_COMPILER/exe/x64/";
#if _DEBUG
	mscDirectory += "Debug";
#else
	mscDirectory += "Release";
#endif

	std::string binaryFilepath, hlslFilePath, debugName, binaryDir;
	auto get_shader_filepath_strings = [&](const std::string& _binaryFilepath) -> void
	{
		binaryFilepath = _binaryFilepath;
		hlslFilePath = binaryFilepath;
		hlslFilePath.replace(hlslFilePath.find("bin"), 3, "HLSL");
		hlslFilePath.replace(hlslFilePath.find("spv"), 3, "hlsl");
		debugName = binaryFilepath.substr(binaryFilepath.find_last_of('/') + 1);
		binaryDir = binaryFilepath.substr(0, binaryFilepath.find_last_of('/'));
	};

	//Basic
	graphics::RenderPipeline::CreateInfo basicPipelineCI;
	basicPipelineCI.shaderCreateInfo.resize(2);

	get_shader_filepath_strings("res/shaders/bin/basic.vert.spv");
	basicPipelineCI.shaderCreateInfo[0].debugName = _strdup(debugName.c_str());
	basicPipelineCI.shaderCreateInfo[0].device = m_Device;
	basicPipelineCI.shaderCreateInfo[0].stage = Shader::StageBit::VERTEX_BIT;
	basicPipelineCI.shaderCreateInfo[0].entryPoint = "main";
	basicPipelineCI.shaderCreateInfo[0].binaryFilepath = _strdup(binaryFilepath.c_str());
	basicPipelineCI.shaderCreateInfo[0].binaryCode = {};
	basicPipelineCI.shaderCreateInfo[0].recompileArguments = {
		_strdup(mscDirectory.c_str()), _strdup(hlslFilePath.c_str()), _strdup(binaryDir.c_str()), { "../GEAR_CORE/dep/MIRU/MIRU_SHADER_COMPILER/shaders/includes" },
		nullptr, "6_4", {}, true, true, nullptr, nullptr, nullptr, false, false };

	get_shader_filepath_strings("res/shaders/bin/basic.frag.spv");
	basicPipelineCI.shaderCreateInfo[1].debugName = _strdup(debugName.c_str());
	basicPipelineCI.shaderCreateInfo[1].device = m_Device;
	basicPipelineCI.shaderCreateInfo[1].stage = Shader::StageBit::FRAGMENT_BIT;
	basicPipelineCI.shaderCreateInfo[1].entryPoint = "main";
	basicPipelineCI.shaderCreateInfo[1].binaryFilepath = _strdup(binaryFilepath.c_str());
	basicPipelineCI.shaderCreateInfo[1].binaryCode = {};
	basicPipelineCI.shaderCreateInfo[1].recompileArguments = {
		_strdup(mscDirectory.c_str()), _strdup(hlslFilePath.c_str()), _strdup(binaryDir.c_str()), { "../GEAR_CORE/dep/MIRU/MIRU_SHADER_COMPILER/shaders/includes" },
		nullptr, "6_4", {}, true, true, nullptr, nullptr, nullptr, false, false };

	basicPipelineCI.viewportState.viewports = { { 0.0f, 0.0f, viewportWidth, viewportHeight, 0.0f, 1.0f } };
	basicPipelineCI.viewportState.scissors = { { { 0, 0 },{ (uint32_t)viewportWidth, (uint32_t)viewportHeight } } };
	basicPipelineCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::BACK_BIT, FrontFace::COUNTER_CLOCKWISE, false, 0.0f, 0.0, 0.0f, 1.0f };
	basicPipelineCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, false, false };
	basicPipelineCI.depthStencilState = { true, true, CompareOp::LESS, false, false, {}, {}, 0.0f, 1.0f };
	basicPipelineCI.colourBlendState = { false, LogicOp::COPY, { {true, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD,
		BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD, (ColourComponentBit)15 } }, { 0.0f, 0.0f, 0.0f, 0.0f } };
	basicPipelineCI.renderPass = renderPass;
	basicPipelineCI.subpassIndex = 0;
	m_RenderPipelines["basic"] = gear::CreateRef<graphics::RenderPipeline>(&basicPipelineCI);

	//Cube
	graphics::RenderPipeline::CreateInfo cubePipelineCI;
	cubePipelineCI.shaderCreateInfo.resize(2);

	get_shader_filepath_strings("res/shaders/bin/cube.vert.spv");
	cubePipelineCI.shaderCreateInfo[0].debugName = _strdup(debugName.c_str());
	cubePipelineCI.shaderCreateInfo[0].device = m_Device;
	cubePipelineCI.shaderCreateInfo[0].stage = Shader::StageBit::VERTEX_BIT;
	cubePipelineCI.shaderCreateInfo[0].entryPoint = "main";
	cubePipelineCI.shaderCreateInfo[0].binaryFilepath = _strdup(binaryFilepath.c_str());
	cubePipelineCI.shaderCreateInfo[0].binaryCode = {};
	cubePipelineCI.shaderCreateInfo[0].recompileArguments = {
		_strdup(mscDirectory.c_str()), _strdup(hlslFilePath.c_str()), _strdup(binaryDir.c_str()), { "../GEAR_CORE/dep/MIRU/MIRU_SHADER_COMPILER/shaders/includes" },
		nullptr, "6_4", {}, true, true, nullptr, nullptr, nullptr, false, false };

	get_shader_filepath_strings("res/shaders/bin/cube.frag.spv");
	cubePipelineCI.shaderCreateInfo[1].debugName = _strdup(debugName.c_str());
	cubePipelineCI.shaderCreateInfo[1].device = m_Device;
	cubePipelineCI.shaderCreateInfo[1].stage = Shader::StageBit::FRAGMENT_BIT;
	cubePipelineCI.shaderCreateInfo[1].entryPoint = "main";
	cubePipelineCI.shaderCreateInfo[1].binaryFilepath = _strdup(binaryFilepath.c_str());
	cubePipelineCI.shaderCreateInfo[1].binaryCode = {};
	cubePipelineCI.shaderCreateInfo[1].recompileArguments = {
		_strdup(mscDirectory.c_str()), _strdup(hlslFilePath.c_str()), _strdup(binaryDir.c_str()), { "../GEAR_CORE/dep/MIRU/MIRU_SHADER_COMPILER/shaders/includes" },
		nullptr, "6_4", {}, true, true, nullptr, nullptr, nullptr, false, false };

	cubePipelineCI.viewportState.viewports = { { 0.0f, 0.0f, viewportWidth, viewportHeight, 0.0f, 1.0f } };
	cubePipelineCI.viewportState.scissors = { { { 0, 0 },{ (uint32_t)viewportWidth, (uint32_t)viewportHeight } } };
	cubePipelineCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::NONE_BIT, FrontFace::COUNTER_CLOCKWISE, false, 0.0f, 0.0, 0.0f, 1.0f };
	cubePipelineCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, false, false };
	cubePipelineCI.depthStencilState = { true, true, CompareOp::LESS, false, false, {}, {}, 0.0f, 1.0f };
	cubePipelineCI.colourBlendState = { false, LogicOp::COPY, { {true, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD,
		BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD, (ColourComponentBit)15 } }, { 0.0f, 0.0f, 0.0f, 0.0f } };
	cubePipelineCI.renderPass = renderPass;
	cubePipelineCI.subpassIndex = 0;
	m_RenderPipelines["cube"] = gear::CreateRef<graphics::RenderPipeline>(&cubePipelineCI);
}

void Renderer::ClearupRenderPipelines()
{
	for (auto& renderPipeline : m_RenderPipelines)
	{
		for (auto& shaderCI : renderPipeline.second->m_CI.shaderCreateInfo)
		{
			free((void*)shaderCI.debugName);
			free((void*)shaderCI.binaryFilepath);
			free((void*)shaderCI.recompileArguments.mscDirectory);
			free((void*)shaderCI.recompileArguments.hlslFilepath);
			free((void*)shaderCI.recompileArguments.outputDirectory);
		}
	}
}

void Renderer::Submit(const gear::Ref<Model>& obj)
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
		m_TransCmdBuffer->Reset(0, false);
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
		m_CmdBuffer->Reset(2, false);
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

	if(!builtDescPoolsAndSets)
	{
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
		builtDescPoolsAndSets = true;
	}

	//Record Present CmdBuffers
	m_DrawFences[m_FrameIndex]->Wait();
	{
		m_CmdBuffer->Reset(m_FrameIndex, false);
		m_CmdBuffer->Begin(m_FrameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
		m_CmdBuffer->BeginRenderPass(m_FrameIndex, m_Framebuffers[m_FrameIndex], { {0.25f, 0.25f, 0.25f, 1.0f}, {1.0f, 0} });

		for (auto& obj : m_RenderQueue)
		{
			const miru::Ref<miru::crossplatform::Pipeline>& pipeline = m_RenderPipelines[obj->GetPipelineName()]->GetPipeline();
			const std::vector<miru::Ref<miru::crossplatform::DescriptorSetLayout>>& descriptorSetLayouts = m_RenderPipelines[obj->GetPipelineName()]->GetDescriptorSetLayouts();

			m_CmdBuffer->BindPipeline(m_FrameIndex, pipeline);

			std::vector<Ref<DescriptorSet>> bindDescriptorSets;
			for (size_t descCount = 0; descCount < descriptorSetLayouts.size(); descCount++)
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
			m_CmdBuffer->BindDescriptorSets(m_FrameIndex, bindDescriptorSets, pipeline);

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