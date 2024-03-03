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
			struct PostProcessingInfo
			{
				struct Bloom
				{
					Ref<Uniformbuffer<UniformBufferStructures::BloomInfo>> UB;
					uint32_t width;
					uint32_t height;
					uint32_t levels;
					miru::base::ImageRef prefilterOutputImage;
					std::vector<miru::base::ImageViewRef> imageViews;
					miru::base::SamplerRef sampler;
				} bloom;
				struct HDRSettings
				{
					Ref<Uniformbuffer<UniformBufferStructures::HDRInfo>> UB;
				} hdrSettings;
			};
			struct DefaultObjects
			{
				Ref<Uniformbuffer<UniformBufferStructures::Lights>> emptyLightsUB;
				Ref<Uniformbuffer<UniformBufferStructures::ProbeInfo>> emptyProbeUB;
				Ref<Texture> black2DTexture;
				Ref<Texture> black2DArrayTexture;
				Ref<Texture> blackCubeTexture;
				Ref<Texture> blackCubeArrayTexture;
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
			std::vector<miru::base::SemaphoreRef> m_AcquireSemaphores;
			std::vector<miru::base::SemaphoreRef> m_SubmitSemaphores;

			//Renderering Objects
			Ref<objects::Camera> m_MainRenderCamera;
			Ref<objects::Camera> m_TextCamera;
			std::vector<Ref<objects::Camera>> m_AllCameras;
			std::vector<Ref<objects::Light>> m_Lights;
			Ref<objects::Skybox> m_Skybox;
			std::vector<Ref<objects::Model>> m_ModelQueue;
			std::vector<Ref<objects::Model>> m_TextQueue;
			std::vector<Ref<Texture>> m_TextureUploadQueue;
			ui::UIContext* m_UIContext = nullptr;
			PostProcessingInfo m_PostProcessingInfo;

			static DefaultObjects s_DefaultObjects;

			uint32_t m_FrameIndex = 0;
			uint32_t m_FrameCount = 0;
			uint32_t m_SwapchainImageIndex = 0;
			uint32_t m_SwapchainImageCount = 0;

			bool m_ReloadTextures = false;
			bool m_DebugRendering = false;

		public:
			Renderer(CreateInfo* pCreateInfo);
			virtual ~Renderer();

		private:
			void InitialiseRenderPipelines(const Ref<RenderSurface>& renderSurface);
			void UninitialiseRenderPipelines();

			void InitialiseDefaultObjects();
			void UninitialiseDefaultObjects();

		public:
			void SubmitRenderSurface(const Ref<RenderSurface>& renderSurface);
			void SubmitCamera(const Ref<objects::Camera>& camera, uint32_t usage);
			void SubmitLight(const Ref<objects::Light>& lights);
			void SubmitSkybox(const Ref<objects::Skybox>& skybox);
			void SubmitModel(const Ref<objects::Model>& obj);
			void SubmitTextLine(const Ref<objects::Model>& obj);
			void SubmitTexturesForUpload(const std::vector<Ref<Texture>>& textures);
			void SubmitUIContext(ui::UIContext* uiContext);

		private:
			void AcquireNextImage();
			void Draw();
			void Present();

		public:
			void Execute();

		public:
			void RecompileRenderPipelineShaders();
			void ReloadRenderPipelines();
			void ReloadTextures();

			inline miru::base::ContextRef GetContext() { return m_Context; }
			inline void* GetDevice() { return m_Device; }
			inline Ref<Window> GetWindow() { return m_CI.window; }
			static inline std::map<std::string, Ref<graphics::RenderPipeline>>& GetRenderPipelines() { return s_RenderPipelines; }
			inline Ref<RenderSurface> GetRenderSurface() { return m_RenderSurface; }
			inline Ref<objects::Camera> GetCamera() { return m_MainRenderCamera; }
			inline Ref<objects::Camera> GetTextCamera() { return m_TextCamera; }
			inline const std::vector<Ref<objects::Light>>& GetLights() { return m_Lights; }
			inline const Ref<objects::Skybox>& GetSkybox() { return m_Skybox; }
			inline const std::vector<Ref<objects::Model>>& GetModelQueue() const { return m_ModelQueue; }
			inline const std::vector<Ref<objects::Model>>& GetTextQueue() const { return m_TextQueue; }
			inline std::vector<Ref<Texture>>& GetTextureUploadQueue() { return m_TextureUploadQueue; }
			inline const RenderGraph& GetRenderGraph() const { return m_RenderGraph; }
			inline RenderGraph& GetRenderGraph() { return m_RenderGraph; }
			inline PostProcessingInfo& GetPostProcessingInfo() { return m_PostProcessingInfo; }
			static inline const DefaultObjects& GetDefaultObjects() { return s_DefaultObjects; }
			inline void SetDebugRendering(bool debugRendeing) { m_DebugRendering = debugRendeing; }

			inline const uint32_t& GetFrameIndex() const { return m_FrameIndex; }
			inline const uint32_t& GetFrameCount() const { return m_FrameCount; }
			inline const uint32_t& GetSwapchainImageIndex() const { return m_SwapchainImageIndex; }
			inline const uint32_t& GetSwapchainImageCount() const { return m_SwapchainImageCount; }

		};
	}
}