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

Node::Node(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if (m_CI.submitCmdBuffer)
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

Node::~Node()
{
}

void Node::Execute()
{
	if (m_CI.resetCmdBuffer)
	{
		m_CI.cmdBuffer->Reset(m_CI.cmdBufferIndex, false);
		m_CI.cmdBuffer->Begin(m_CI.cmdBufferIndex, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
	}

	if (!m_CI.skipTask)
	{
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
		default:
			break;
		}
	}

	if (m_CI.submitCmdBuffer)
	{
		std::vector<Ref<Semaphore>> waitSrcSemaphores;
		std::vector<PipelineStageBit> waitSrcPipelineStages;
		
		for (const auto& lastSubmitNodes : LastSubmitNodes(m_CI.srcPipelineStages, m_CI.srcNodes))
		{
			waitSrcPipelineStages.push_back(lastSubmitNodes.first);
			waitSrcSemaphores.push_back(lastSubmitNodes.second->m_SignalSemaphore);
		}

		m_CI.cmdBuffer->End(m_CI.cmdBufferIndex);
		m_CI.cmdBuffer->Submit({ m_CI.cmdBufferIndex }, waitSrcSemaphores, waitSrcPipelineStages, { m_SignalSemaphore }, m_CmdBufferFence);
	}
}

std::vector<std::pair<PipelineStageBit, Ref<Node>>> Node::LastSubmitNodes(std::vector<PipelineStageBit>& srcPipelineStages, std::vector<Ref<Node>>& srcNodes)
{
	std::vector<std::pair<PipelineStageBit, Ref<Node>>> nodes;
	for (size_t i = 0; i < srcNodes.size(); i++)
	{
		Ref<Node>& node = srcNodes[i];
		PipelineStageBit pipelineStage = srcPipelineStages[i];

		if (node->m_CI.submitCmdBuffer)
		{
			nodes.push_back({ pipelineStage, node });
		}
		else
		{
			auto& sub_first_nodes = LastSubmitNodes(node->m_CI.srcPipelineStages, node->m_CI.srcNodes);
			nodes.insert(nodes.end(), sub_first_nodes.begin(), sub_first_nodes.end());
		}
	}
	return nodes;
};

void Node::TransitionResources()
{
	TransitionResourcesTaskInfo* transResourcesTI = reinterpret_cast<TransitionResourcesTaskInfo*>(m_CI.pTaskInfo);
	m_CI.cmdBuffer->PipelineBarrier(m_CI.cmdBufferIndex, transResourcesTI->srcPipelineStage, transResourcesTI->dstPipelineStage, DependencyBit::NONE_BIT, transResourcesTI->barriers);
}

void Node::UploadResources()
{
	UploadResourceTaskInfo* uploadResourcesTI = reinterpret_cast<UploadResourceTaskInfo*>(m_CI.pTaskInfo);

	if (uploadResourcesTI->camera)
		uploadResourcesTI->camera->GetUB()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->cameraForce);

	if (uploadResourcesTI->fontCamera)
		uploadResourcesTI->fontCamera->GetUB()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->fontCameraForce);

	if (uploadResourcesTI->skybox)
		uploadResourcesTI->skybox->GetUB()->Upload(m_CI.cmdBuffer, m_CI.cmdBufferIndex, uploadResourcesTI->skyboxForce);

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

