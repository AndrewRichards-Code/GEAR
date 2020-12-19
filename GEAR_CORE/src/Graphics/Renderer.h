#pragma once

#include "gear_core_common.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/RenderPipeline.h"
#include "Objects/Camera.h"
#include "Objects/Light.h"
#include "Objects/Skybox.h"
#include "Objects/Model.h"

namespace gear 
{
namespace graphics 
{
	class Renderer
	{
	private:
		//Context and Device
		void* m_Device;
		miru::Ref<miru::crossplatform::Context> m_Context;

		//Cmd Pools and CmdBuffers
		miru::Ref<miru::crossplatform::CommandPool> m_CmdPool;
		miru::crossplatform::CommandPool::CreateInfo m_CmdPoolCI;
		miru::Ref<miru::crossplatform::CommandBuffer> m_CmdBuffer;
		miru::crossplatform::CommandBuffer::CreateInfo m_CmdBufferCI;

		miru::Ref<miru::crossplatform::CommandPool> m_TransCmdPool;
		miru::crossplatform::CommandPool::CreateInfo m_TransCmdPoolCI;
		miru::Ref<miru::crossplatform::CommandBuffer> m_TransCmdBuffer;
		miru::crossplatform::CommandBuffer::CreateInfo m_TransCmdBufferCI;

		//Descriptor Pool and Sets
		miru::Ref<miru::crossplatform::DescriptorPool> m_DescPool;
		miru::crossplatform::DescriptorPool::CreateInfo m_DescPoolCI;

		std::map<gear::Ref<graphics::RenderPipeline>, miru::Ref<miru::crossplatform::DescriptorSet>> m_DescSetPerView;
		std::map<gear::Ref<objects::Model>, miru::Ref<miru::crossplatform::DescriptorSet>> m_DescSetPerModel;
		std::map<gear::Ref<objects::Material>, miru::Ref<miru::crossplatform::DescriptorSet>> m_DescSetPerMaterial;

		bool m_BuiltDescPoolsAndSets = false;
		bool m_ReloadTextures = false;

		//Renderering Objects
		std::map<std::string, gear::Ref<graphics::RenderPipeline>> m_RenderPipelines;
		const miru::Ref<miru::crossplatform::Framebuffer>* m_Framebuffers;
		gear::Ref<objects::Camera> m_Camera;
		std::vector<gear::Ref<objects::Light>> m_Lights;
		gear::Ref<objects::Skybox> m_Skybox;
		std::vector<gear::Ref<objects::Model>> m_RenderQueue;

		//Present Synchronisation Primitives
		std::vector<miru::Ref<miru::crossplatform::Fence>> m_DrawFences;
		miru::crossplatform::Fence::CreateInfo m_DrawFenceCI;
		std::vector<miru::Ref<miru::crossplatform::Semaphore>>m_AcquireSemaphores;
		miru::crossplatform::Semaphore::CreateInfo m_AcquireSemaphoreCI;
		std::vector<miru::Ref<miru::crossplatform::Semaphore>>m_SubmitSemaphores;
		miru::crossplatform::Semaphore::CreateInfo m_SubmitSemaphoreCI;

		uint32_t m_FrameIndex = 0;
		uint32_t m_FrameCount = 0;

	public:
		Renderer(const miru::Ref<miru::crossplatform::Context>& context);
		virtual ~Renderer();

		void InitialiseRenderPipelines(const std::vector<std::string>& filepaths, float viewportWidth, float viewportHeight, miru::crossplatform::Image::SampleCountBit samples, const miru::Ref<miru::crossplatform::RenderPass>& renderPass);
		
		void SubmitFramebuffer(const miru::Ref<miru::crossplatform::Framebuffer>* framebuffers);
		void SubmitCamera(gear::Ref<objects::Camera>& camera);
		void SubmitLights(std::vector<gear::Ref<objects::Light>>& lights);
		void SubmitSkybox(const gear::Ref<objects::Skybox>& skybox);
		void SubmitModel(const gear::Ref<objects::Model>& obj);

		void Upload(bool forceUploadCamera, bool forceUploadLights, bool forceUploadSkybox, bool forceUploadMeshes);
		void Flush();
		void Present(const miru::Ref<miru::crossplatform::Swapchain>& swapchain, bool& windowResize);

		void ResizeRenderPipelineViewports(uint32_t width, uint32_t height);
		void RecompileRenderPipelineShaders();
		void ReloadTextures();

		inline std::vector<gear::Ref<objects::Model>>& GetRenderQueue() { return m_RenderQueue; };
		inline const miru::Ref<miru::crossplatform::CommandBuffer>& GetCmdBuffer() { return m_CmdBuffer; };
		inline const std::map<std::string, gear::Ref<graphics::RenderPipeline>>& GetRenderPipelines() const { return m_RenderPipelines; }
	};
}
}