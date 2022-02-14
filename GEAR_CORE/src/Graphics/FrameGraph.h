#pragma once

#include "gear_core_common.h"
#include "Graphics/Renderer.h"
#include "Graphics/ImageProcessing.h"

namespace gear
{
	namespace objects
	{
		class Camera;
		class Skybox;
		class Light;
		class Model;
	}

	namespace graphics
	{
		class Renderer;

		class GEAR_API GPUTask
		{
		public:
			enum class Task : uint32_t
			{
				NONE,
				UPLOAD_RESOURCES,
				TRANSITION_RESOURCES,
				GRAPHICS_RENDER_PASS_BEGIN,
				GRAPHICS_RENDER_PASS_END,
				GRAPHICS_NEXT_SUBPASS,
				COMPUTE_DISPATCH,
				RENDERER_FUNCTION,
				IMAGE_PROCESSING_FUNCTION_1,
				IMAGE_PROCESSING_FUNCTION_2,
			};
			enum class CommandBufferBasicControlsBit : uint32_t
			{
				NONE	= 0x00000000,
				RESET	= 0x00000001,
				BEGIN	= 0x00000002,
				END		= 0x00000004,
				SUBMIT	= 0x00000008,

				RESET_BEGIN = RESET | BEGIN,
				END_SUBMIT = END | SUBMIT,
				RESET_BEGIN_END_SUBMIT = RESET | BEGIN | END | SUBMIT,
			};
			
			struct UploadResourceTaskInfo
			{
				Ref<objects::Camera>					camera;
				bool									cameraForce;
				Ref<objects::Camera>					textCamera;
				bool									textCameraForce;
				Ref<objects::Skybox>					skybox;
				bool									skyboxForce;
				std::vector<Ref<objects::Light>>		lights;
				bool									lightsForce;
				std::vector<Ref<objects::Model>>		models;
				bool									modelsForce;
				bool									materialsForce;
			};
			struct TransitionResourcesTaskInfo
			{
				miru::crossplatform::PipelineStageBit			srcPipelineStage;
				miru::crossplatform::PipelineStageBit			dstPipelineStage;
				std::vector<Ref<miru::crossplatform::Barrier>>	barriers;
			};
			struct GraphicsRenderPassBeginTaskInfo
			{
				Ref<miru::crossplatform::Framebuffer>				framebuffer;
				std::vector<miru::crossplatform::Image::ClearValue>	clearValues;
			};
			struct GraphicsRenderPassEndTaskInfo
			{
				uint32_t unused;
			};
			struct GraphicsNextSubpassTaskInfo
			{
				uint32_t unused;
			};
			struct RendererFunctionTaskInfo
			{
				Renderer*							pRenderer;
				Renderer::PFN_RendererFunction		pfn;
				Renderer::DescriptorPoolAndSets*	pDescPoolAndSets;
			};
			struct ImageProcessingFunctionTaskInfo1
			{
				ImageProcessing::PFN_ImageProcessing1	pfn;
				ImageProcessing::TextureResourceInfo	tri1;
			};
			struct ImageProcessingFunctionTaskInfo2
			{
				ImageProcessing::PFN_ImageProcessing2	pfn;
				ImageProcessing::TextureResourceInfo	tri1;
				ImageProcessing::TextureResourceInfo	tri2;
			};

			struct CreateInfo
			{
				std::string											debugName;
				Task												task;
				void*												pTaskInfo;
				std::vector<Ref<GPUTask>>							srcGPUTasks;
				std::vector<miru::crossplatform::PipelineStageBit>	srcPipelineStages;
				Ref<miru::crossplatform::CommandBuffer>				cmdBuffer;
				uint32_t											cmdBufferIndex;
				CommandBufferBasicControlsBit						cmdBufferControls;
				bool												resetCmdBufferReleaseResource;
				miru::crossplatform::CommandBuffer::UsageBit		beginCmdBufferUsage;
				bool												skipTask;
			};

		private:
			CreateInfo m_CI;

			Ref<miru::crossplatform::Fence> m_CmdBufferFence;
			miru::crossplatform::Fence::CreateInfo m_CmdBufferFenceCI;

			Ref<miru::crossplatform::Semaphore> m_SignalSemaphore;
			miru::crossplatform::Semaphore::CreateInfo m_SignalSemaphoreCI;


		public:
			GPUTask(CreateInfo* pCreateInfo);
			~GPUTask();

			void Execute();

			inline const CreateInfo& GetCreateInfo() const { return m_CI; }

			inline const Ref<miru::crossplatform::Fence>& GetFence() const { return m_CmdBufferFence; }
			inline const Ref<miru::crossplatform::Semaphore>& GetSemaphore() const { return m_SignalSemaphore; }

		private:
			std::vector<std::pair<miru::crossplatform::PipelineStageBit, Ref<GPUTask>>> LastSubmitGPUTasks(
				std::vector<miru::crossplatform::PipelineStageBit>& srcPipelineStages, std::vector<Ref<GPUTask>>& srcGPUTasks);
			void TransitionResources();
			void UploadResources();
			void GraphicsRenderPassBegin();
			void GraphicsRenderPassEnd();
			void GraphicsNextSubpass();
			void RendererFunction();
			void ImageProcessingFunction1();
			void ImageProcessingFunction2();

		};
	}
}
