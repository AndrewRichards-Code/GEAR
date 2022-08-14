#pragma once
#include "Graphics/Rendering/RenderGraph.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Window.h"
#include "Objects/Camera.h"
#include "Objects/Light.h"
#include "Objects/Skybox.h"
#include "Objects/Model.h"

namespace gear 
{
	namespace ui
	{
		class UIContext;
	}
	namespace graphics::rendering
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

		private:
			CreateInfo m_CI;

			//Context and Device
			void* m_Device;
			miru::base::ContextRef m_Context;

			Ref<RenderSurface> m_RenderSurface;
			static std::map<std::string, Ref<graphics::RenderPipeline>> s_RenderPipelines;

			RenderGraph m_RenderGraph;

			//Present Synchronisation Primitives
			std::vector<miru::base::FenceRef> m_DrawFences;
			miru::base::Fence::CreateInfo m_DrawFenceCI;
			miru::base::SemaphoreRef m_AcquireSemaphore;
			miru::base::Semaphore::CreateInfo m_AcquireSemaphoreCI;
			miru::base::SemaphoreRef m_SubmitSemaphore;
			miru::base::Semaphore::CreateInfo m_SubmitSemaphoreCI;

			//Renderering Objects
			Ref<objects::Camera> m_MainRenderCamera;
			Ref<objects::Camera> m_TextCamera;
			std::vector<Ref<objects::Camera>> m_AllCameras;
			std::vector<Ref<objects::Light>> m_Lights;
			Ref<objects::Skybox> m_Skybox;
			std::vector<Ref<objects::Model>> m_ModelQueue;
			std::vector<Ref<objects::Model>> m_TextQueue;
			ui::UIContext* m_UIContext = nullptr;

			//Default Objects
			Ref<Uniformbuffer<UniformBufferStructures::Lights>> m_EmptyLightsUB;
			Ref<Texture> m_Black2DTexture;
			Ref<Texture> m_BlackCubeTexture;

			uint32_t m_FrameIndex = 0;
			uint32_t m_FrameCount = 0;
			uint32_t m_SwapchainImageCount = 0;

			bool m_ReloadTextures = false;

		public:
			Renderer(CreateInfo* pCreateInfo);
			virtual ~Renderer();

		private:
			void InitialiseRenderPipelines(const Ref<RenderSurface>& renderSurface);
			void UninitialiseRenderPipelines();

		public:
			void SubmitRenderSurface(const Ref<RenderSurface>& renderSurface);
			void SubmitCamera(const Ref<objects::Camera>& camera, uint32_t usage);
			void SubmitLight(const Ref<objects::Light>& lights);
			void SubmitSkybox(const Ref<objects::Skybox>& skybox);
			void SubmitModel(const Ref<objects::Model>& obj);
			void SubmitTextLine(const Ref<objects::Model>& obj);
			void SubmitUIContext(ui::UIContext* uiContext);

		private:
			void AcquireNextImage();
			void Draw();
			void Present();

		public:
			void Execute();

		public:
			void RecompileRenderPipelineShaders();
			void ReloadTextures();

			inline miru::base::ContextRef GetContext() { return m_Context; }
			inline void* GetDevice() { return m_Device; }
			inline Ref<Window> GetWindow() { return m_CI.window; }
			static inline std::map<std::string, Ref<graphics::RenderPipeline>> GetRenderPipelines() { return s_RenderPipelines; }
			inline Ref<RenderSurface> GetRenderSurface() { return m_RenderSurface; }
			inline Ref<objects::Camera> GetCamera() { return m_MainRenderCamera; }
			inline const RenderGraph& GetRenderGraph() const { return m_RenderGraph; }

			inline const uint32_t& GetFrameIndex() const { return m_FrameIndex; }
			inline const uint32_t& GetFrameCount() const { return m_FrameCount; }

		};
	}
}