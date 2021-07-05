#pragma once

#include "gear_core_common.h"

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
		class GPUTask
		{
		public:
			enum class Task : uint32_t
			{
				NONE,
				UPLOAD_RESOURCES,
				TRANSITION_RESOURCES,
				GRAPHICS_RENDER_PASS_BEGIN,
				GRAPHICS_RENDER_PASS_END,
				COMPUTE_DISPATCH
			};
			
			struct UploadResourceTaskInfo
			{
				Ref<objects::Camera>					camera;
				bool									cameraForce;
				Ref<objects::Camera>					fontCamera;
				bool									fontCameraForce;
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

			struct CreateInfo
			{
				std::string											debugName;
				Task												task;
				void*												pTaskInfo;
				std::vector<Ref<GPUTask>>								srcGPUTasks;
				std::vector<miru::crossplatform::PipelineStageBit>	srcPipelineStages;
				Ref<miru::crossplatform::CommandBuffer>				cmdBuffer;
				uint32_t											cmdBufferIndex;
				bool												resetCmdBuffer;
				bool												submitCmdBuffer;
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

		};
	}
}
