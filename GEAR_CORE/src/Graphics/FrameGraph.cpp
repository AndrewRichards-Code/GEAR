#include "gear_core_common.h"

#include "FrameGraph.h"
#include "Graphics/AllocatorManager.h"

#include "Objects/Camera.h"
#include "Objects/Skybox.h"
#include "Objects/Light.h"
#include "Objects/Model.h"
#include "Objects/Material.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

GPUTask::GPUTask(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if ((m_CI.cmdBufferControls & CommandBufferBasicControlsBit::SUBMIT) == CommandBufferBasicControlsBit::SUBMIT)
	{
		m_CmdBufferFenceCI.debugName = "GEAR_CORE_Fence_" + m_CI.debugName;
		m_CmdBufferFenceCI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
		m_CmdBufferFenceCI.signaled = false;
		m_CmdBufferFenceCI.timeout = UINT64_MAX;
		m_CmdBufferFence = Fence::Create(&m_CmdBufferFenceCI);

		m_SignalSemaphoreCI.debugName = "GEAR_CORE_Semaphore_" + m_CI.debugName;
		m_SignalSemaphoreCI.device = m_CmdBufferFenceCI.device;
		m_SignalSemaphore = Semaphore::Create(&m_SignalSemaphoreCI);
	}
}

GPUTask::~GPUTask()
{
}

void GPUTask::Execute()
{
	if ((m_CI.cmdBufferControls & CommandBufferBasicControlsBit::RESET) == CommandBufferBasicControlsBit::RESET)
	{
		m_CI.cmdBuffer->Reset(m_CI.cmdBufferIndex, m_CI.resetCmdBufferReleaseResource);
	}
	if ((m_CI.cmdBufferControls & CommandBufferBasicControlsBit::BEGIN) == CommandBufferBasicControlsBit::BEGIN)
	{
		m_CI.cmdBuffer->Begin(m_CI.cmdBufferIndex, m_CI.beginCmdBufferUsage);
	}

	if (!m_CI.skipTask)
	{
		if (m_CI.cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().queueType == CommandPool::QueueType::GRAPHICS
			|| m_CI.cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().queueType == CommandPool::QueueType::COMPUTE)
		{
			m_CI.cmdBuffer->BeginDebugLabel(m_CI.cmdBufferIndex, m_CI.debugName);
		}

		switch (m_CI.task)
		{
		case Task::UPLOAD_RESOURCES:
		{
			UploadResources();
			break;
		}
		case Task::TRANSITION_RESOURCES:
		{
			TransitionResources();
			break;
		}
		case Task::GRAPHICS_RENDER_PASS_BEGIN:
		{
			GraphicsRenderPassBegin();
			break;
		}
		case Task::GRAPHICS_RENDER_PASS_END:
		{
			GraphicsRenderPassEnd();
			break;
		}
		case Task::GRAPHICS_NEXT_SUBPASS:
		{
			GraphicsNextSubpass();
			break;
		}
		case Task::RENDERER_FUNCTION:
		{
			RendererFunction();
			break;
		}
		default:
			break;
		}

		if (m_CI.cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().queueType == CommandPool::QueueType::GRAPHICS
			|| m_CI.cmdBuffer->GetCreateInfo().pCommandPool->GetCreateInfo().queueType == CommandPool::QueueType::COMPUTE)
		{
			m_CI.cmdBuffer->EndDebugLabel(m_CI.cmdBufferIndex);
		}
	}
	
	if ((m_CI.cmdBufferControls & CommandBufferBasicControlsBit::END) == CommandBufferBasicControlsBit::END)
	{
		m_CI.cmdBuffer->End(m_CI.cmdBufferIndex);
	}
	if ((m_CI.cmdBufferControls & CommandBufferBasicControlsBit::SUBMIT) == CommandBufferBasicControlsBit::SUBMIT)
	{
		std::vector<Ref<Semaphore>> waitSrcSemaphores;
		std::vector<PipelineStageBit> waitSrcPipelineStages;
		
		for (const auto& lastSubmitNodes : LastSubmitGPUTasks(m_CI.srcPipelineStages, m_CI.srcGPUTasks))
		{
			waitSrcPipelineStages.push_back(lastSubmitNodes.first);
			waitSrcSemaphores.push_back(lastSubmitNodes.second->m_SignalSemaphore);
		}

		m_CI.cmdBuffer->Submit({ m_CI.cmdBufferIndex }, waitSrcSemaphores, waitSrcPipelineStages, { m_SignalSemaphore }, m_CmdBufferFence);
	}
}

std::vector<std::pair<PipelineStageBit, Ref<GPUTask>>> GPUTask::LastSubmitGPUTasks(std::vector<PipelineStageBit>& srcPipelineStages, std::vector<Ref<GPUTask>>& srcGPUTasks)
{
	std::vector<std::pair<PipelineStageBit, Ref<GPUTask>>> gpuTasks;
	for (size_t i = 0; i < srcGPUTasks.size(); i++)
	{
		Ref<GPUTask>& gpuTask = srcGPUTasks[i];
		PipelineStageBit pipelineStage = srcPipelineStages[i];

		if ((gpuTask->m_CI.cmdBufferControls & CommandBufferBasicControlsBit::SUBMIT) == CommandBufferBasicControlsBit::SUBMIT)
		{
			gpuTasks.push_back({ pipelineStage, gpuTask });
		}
		else
		{
			auto& sub_first_gpu_tasks = LastSubmitGPUTasks(gpuTask->m_CI.srcPipelineStages, gpuTask->m_CI.srcGPUTasks);
			gpuTasks.insert(gpuTasks.end(), sub_first_gpu_tasks.begin(), sub_first_gpu_tasks.end());
		}
	}
	return gpuTasks;
};

void GPUTask::TransitionResources()
{
	TransitionResourcesTaskInfo* transResourcesTI = reinterpret_cast<TransitionResourcesTaskInfo*>(m_CI.pTaskInfo);
	m_CI.cmdBuffer->PipelineBarrier(m_CI.cmdBufferIndex, transResourcesTI->srcPipelineStage, transResourcesTI->dstPipelineStage, DependencyBit::NONE_BIT, transResourcesTI->barriers);
}

void GPUTask::UploadResources()
{
	UploadResourceTaskInfo* uploadResourcesTI = reinterpret_cast<UploadResourceTaskInfo*>(m_CI.pTaskInfo);

	if (uploadResourcesTI->camera)
		uploadResourcesTI->camera->GetUB()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->cameraForce);

	if (uploadResourcesTI->textCamera)
		uploadResourcesTI->textCamera->GetUB()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->textCameraForce);

	if (uploadResourcesTI->skybox)
	{
		uploadResourcesTI->skybox->GetUB()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->skyboxForce);
		uploadResourcesTI->skybox->GetTexture()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex);
	}

	for (auto& light : uploadResourcesTI->lights)
		light->GetUB()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->lightsForce);

	for (auto& model : uploadResourcesTI->models)
	{
		for (auto& vb : model->GetMesh()->GetVertexBuffers())
			vb->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->modelsForce);
		for (auto& ib : model->GetMesh()->GetIndexBuffers())
			ib->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->modelsForce);

		model->GetUB()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->modelsForce);
		
		for (auto& material : model->GetMesh()->GetMaterials())
		{
			material->GetUB()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->materialsForce);
			
			for (auto& texture : material->GetTextures())
			{
				texture.second->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->materialsForce);
			}
		}
	}
}

void GPUTask::GraphicsRenderPassBegin()
{
	GraphicsRenderPassBeginTaskInfo* graphicsRenderPassBeginTI = reinterpret_cast<GraphicsRenderPassBeginTaskInfo*>(m_CI.pTaskInfo);
	m_CI.cmdBuffer->BeginRenderPass(m_CI.cmdBufferIndex, graphicsRenderPassBeginTI->framebuffer, graphicsRenderPassBeginTI->clearValues);
}

void GPUTask::GraphicsRenderPassEnd()
{
	m_CI.cmdBuffer->EndRenderPass(m_CI.cmdBufferIndex);
}

void GPUTask::GraphicsNextSubpass()
{
	m_CI.cmdBuffer->NextSubpass(m_CI.cmdBufferIndex);
}

void GPUTask::RendererFunction()
{
	RendererFunctionTaskInfo* rendererFunctionTI = reinterpret_cast<RendererFunctionTaskInfo*>(m_CI.pTaskInfo);
	Renderer* pRenderer = rendererFunctionTI->pRenderer;
	Renderer::DescriptorPoolAndSets descPoolsandSets; 
	if (rendererFunctionTI->pDescPoolAndSets)
		descPoolsandSets = *(rendererFunctionTI->pDescPoolAndSets);
	Renderer::PFN_RendererFunction pfn = rendererFunctionTI->pfn;
	(pRenderer->*pfn)(m_CI.cmdBuffer, m_CI.cmdBufferIndex, descPoolsandSets);
}

