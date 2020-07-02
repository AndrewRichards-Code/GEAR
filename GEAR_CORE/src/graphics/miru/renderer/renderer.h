#pragma once

#include "gear_core_common.h"
#include "mars.h"
#include "graphics/miru/buffer/framebuffer.h"
#include "graphics/miru/buffer/vertexbuffer.h"
#include "graphics/miru/buffer/indexbuffer.h"
#include "graphics/miru/pipeline.h"
#include "objects/model.h"
#include "objects/camera.h"
#include "objects/light.h"

namespace gear {
namespace graphics {
class Renderer
{
private:
	void* m_Device;

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
	std::map<objects::Model*, miru::Ref<miru::crossplatform::DescriptorSet>> m_DescSetObj;
	miru::Ref<miru::crossplatform::DescriptorSet> m_DescSetLight;
	miru::crossplatform::DescriptorSet::CreateInfo m_DescSetCI;

	miru::Ref<miru::crossplatform::Framebuffer>* m_Framebuffers;
	std::deque<objects::Model*> m_RenderQueue;
	objects::Camera* m_Camera;
	std::vector<objects::Light*> m_Lights;

public:
	Renderer(miru::Ref<miru::crossplatform::Context> context);
	virtual ~Renderer();

	virtual void SubmitFramebuffer(miru::Ref<miru::crossplatform::Framebuffer>* framebuffers) { m_Framebuffers = framebuffers; };
	virtual void SubmitCamera(objects::Camera* camera) { m_Camera = camera; };
	virtual void SubmitLights(std::vector<objects::Light*> lights) { m_Lights = lights; };
	virtual void Submit(objects::Model* obj);
	virtual void Flush();

	virtual void UpdateCamera();

	inline std::deque<objects::Model*>& GetRenderQueue() { return m_RenderQueue; };
	inline miru::Ref<miru::crossplatform::CommandBuffer> GetCmdBuffer() { return m_CmdBuffer; };
};
}
}