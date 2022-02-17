#pragma once

#include "gear_core_common.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/RenderPipeline.h"
#include "Objects/Camera.h"
#include "Objects/Light.h"
#include "Objects/Skybox.h"
#include "Objects/Model.h"
#include "UI/UIContext.h"

namespace gear 
{
namespace graphics 
{
	class GEAR_API Renderer
	{
	public:
		struct CreateInfo
		{
			Ref<Window> window;
			bool		shouldCopyToSwapchian;
			bool		shouldDrawExternalUI;
			bool		shouldPresent;
		};
		struct CommandPoolAndBuffers
		{
			Ref<miru::crossplatform::CommandPool> cmdPool;
			miru::crossplatform::CommandPool::CreateInfo cmdPoolCI;
			Ref<miru::crossplatform::CommandBuffer> cmdBuffer;
			miru::crossplatform::CommandBuffer::CreateInfo cmdBufferCI;
		};
		struct DescriptorPoolAndSets
		{
			Ref<miru::crossplatform::DescriptorPool> pool;
			miru::crossplatform::DescriptorPool::CreateInfo poolCI;

			std::map<Ref<graphics::RenderPipeline>, Ref<miru::crossplatform::DescriptorSet>> setPerRenderPipeline;
			std::map<Ref<objects::Model>, Ref<miru::crossplatform::DescriptorSet>> setPerModel;
			std::map<Ref<objects::Material>, Ref<miru::crossplatform::DescriptorSet>> setPerMaterial;
		};

	private:
		CreateInfo m_CI;

		//Context and Device
		void* m_Device;
		Ref<miru::crossplatform::Context> m_Context;
		
		Ref<RenderSurface> m_RenderSurface;
		static std::map<std::string, Ref<graphics::RenderPipeline>> s_RenderPipelines;

		//Cmd Pools and CmdBuffers
		std::map<miru::crossplatform::CommandPool::QueueType, CommandPoolAndBuffers> m_CommandPoolAndBuffers;

		//Descriptor Pool and Sets
		std::vector<DescriptorPoolAndSets> m_DescPoolAndSets;

		//Present Synchronisation Primitives
		std::vector<Ref<miru::crossplatform::Fence>> m_DrawFences;
		miru::crossplatform::Fence::CreateInfo m_DrawFenceCI;
		Ref<miru::crossplatform::Semaphore> m_AcquireSemaphore;
		miru::crossplatform::Semaphore::CreateInfo m_AcquireSemaphoreCI;
		Ref<miru::crossplatform::Semaphore> m_SubmitSemaphore;
		miru::crossplatform::Semaphore::CreateInfo m_SubmitSemaphoreCI;

		//Renderering Objects
		Ref<objects::Camera> m_Camera;
		Ref<objects::Camera> m_TextCamera;
		std::vector<Ref<objects::Light>> m_Lights;
		Ref<objects::Skybox> m_Skybox;
		std::vector<Ref<objects::Model>> m_ModelQueue;
		std::vector<Ref<objects::Model>> m_TextQueue;
		ui::UIContext* m_UIContext = nullptr;

		//Default Objects
		Ref<objects::Light> m_DefaultLight;
		
		uint32_t m_FrameIndex = 0;
		uint32_t m_FrameCount = 0;
		uint32_t m_SwapchainImageCount = 0;

		bool m_ReloadTextures = false;

	public:
		Renderer(CreateInfo* pCreateInfo);
		virtual ~Renderer();

	private:
		void InitialiseRenderPipelines(const Ref<RenderSurface>& renderSurface);
		
	public:
		void SubmitRenderSurface(const Ref<RenderSurface>& renderSurface);
		void SubmitCamera(const Ref<objects::Camera>& camera);
		void SubmitTextCamera(const Ref<objects::Camera>& fontCamera);
		void SubmitLight(const Ref<objects::Light>& lights);
		void SubmitSkybox(const Ref<objects::Skybox>& skybox);
		void SubmitModel(const Ref<objects::Model>& obj);
		void SubmitTextLine(const Ref<objects::Model>& obj);
		void SubmitUIContext(ui::UIContext* uiContext);

	private:
		void AcquireNextImage();
		void Upload();
		void BuildDescriptorSetandPools();
		void Draw();
		void Present();

	public:
		void Execute();

	private:
		void MainRenderLoop(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets);
		void HDRMapping(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets);
		void DrawTextLines(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets);
		void DrawCoordinateAxes(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets);
		void CopyToSwapchain(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets);
		void DrawExternalUI(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets);

	public:
		typedef void(Renderer::*PFN_RendererFunction)(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets);

		void ResizeRenderPipelineViewports(uint32_t width, uint32_t height);
		void RecompileRenderPipelineShaders();
		void ReloadTextures();

		inline Ref<miru::crossplatform::Context> GetContext() { return m_Context; }
		inline void* GetDevice() { return m_Device; }
		inline Ref<Window> GetWindow() { return m_CI.window; }
		inline Ref<RenderSurface> GetRenderSurface() { return m_RenderSurface; }
		inline Ref<objects::Camera> GetCamera() { return m_Camera; }

		inline const uint32_t& GetFrameIndex() const { return m_FrameIndex; }
		inline const uint32_t& GetFrameCount() const { return m_FrameCount; }

	};
}
}