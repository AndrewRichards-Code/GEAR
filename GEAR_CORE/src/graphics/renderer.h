#pragma once

#include "gear_core_common.h"
#include "mars.h"
#include "graphics/framebuffer.h"
#include "graphics/renderpipeline.h"
#include "objects/model.h"
#include "objects/camera.h"
#include "objects/light.h"

namespace gear {
namespace graphics {
class Renderer
{
private:
	void* m_Device;

	miru::Ref<miru::crossplatform::Context> m_Context;

	miru::Ref<miru::crossplatform::CommandPool> m_CmdPool;
	miru::crossplatform::CommandPool::CreateInfo m_CmdPoolCI;
	miru::Ref<miru::crossplatform::CommandBuffer> m_CmdBuffer;
	miru::crossplatform::CommandBuffer::CreateInfo m_CmdBufferCI;

	miru::Ref<miru::crossplatform::CommandPool> m_TransCmdPool;
	miru::crossplatform::CommandPool::CreateInfo m_TransCmdPoolCI;
	miru::Ref<miru::crossplatform::CommandBuffer> m_TransCmdBuffer;
	miru::crossplatform::CommandBuffer::CreateInfo m_TransCmdBufferCI;

	miru::Ref<miru::crossplatform::DescriptorPool> m_DescPool;
	miru::crossplatform::DescriptorPool::CreateInfo m_DescPoolCI;

	std::vector<miru::Ref<miru::crossplatform::DescriptorSetLayout>> m_DescSetLayouts;
	
	miru::Ref<miru::crossplatform::DescriptorSet> m_DescSetCamera;
	std::map<gear::Ref<objects::Model>, miru::Ref<miru::crossplatform::DescriptorSet>> m_DescSetObj;
	miru::Ref<miru::crossplatform::DescriptorSet> m_DescSetLight;
	miru::crossplatform::DescriptorSet::CreateInfo m_DescSetCI;

	std::map<std::string, gear::Ref<graphics::RenderPipeline>> m_RenderPipelines;
	const miru::Ref<miru::crossplatform::Framebuffer>* m_Framebuffers;
	std::deque<gear::Ref<objects::Model>> m_RenderQueue;
	objects::Camera* m_Camera;
	std::vector<objects::Light*> m_Lights;

	uint32_t m_FrameIndex = 0;
	uint32_t m_FrameCount = 0;
	bool builtDescPoolsAndSets = false;

	std::vector<miru::Ref<miru::crossplatform::Fence>> m_DrawFences;
	miru::crossplatform::Fence::CreateInfo m_DrawFenceCI;
	std::vector<miru::Ref<miru::crossplatform::Semaphore>>m_AcquireSemaphores;
	miru::crossplatform::Semaphore::CreateInfo m_AcquireSemaphoreCI;
	std::vector<miru::Ref<miru::crossplatform::Semaphore>>m_SubmitSemaphores;
	miru::crossplatform::Semaphore::CreateInfo m_SubmitSemaphoreCI;

public:
	Renderer(const miru::Ref<miru::crossplatform::Context>& context);
	virtual ~Renderer();

	void InitialiseRenderPipelines(float viewportWidth, float viewportHeight, const miru::Ref<miru::crossplatform::RenderPass>& renderPass);
	void ClearupRenderPipelines();
	virtual void SubmitFramebuffer(const miru::Ref<miru::crossplatform::Framebuffer>* framebuffers) { m_Framebuffers = framebuffers; };
	virtual void SubmitCamera(objects::Camera* camera) { m_Camera = camera; };
	virtual void SubmitLights(std::vector<objects::Light*> lights) { m_Lights = lights; };
	virtual void Submit(const gear::Ref<objects::Model>& obj);
	virtual void Flush();
	virtual void Present(const miru::Ref<miru::crossplatform::Swapchain>& swapchain, bool& windowResize);

	virtual void UpdateCamera();

	inline std::deque<gear::Ref<objects::Model>>& GetRenderQueue() { return m_RenderQueue; };
	inline const miru::Ref<miru::crossplatform::CommandBuffer>& GetCmdBuffer() { return m_CmdBuffer; };
	inline const std::map<std::string, gear::Ref<graphics::RenderPipeline>>& GetRenderPipelines() const { return m_RenderPipelines;  }
};
}
}