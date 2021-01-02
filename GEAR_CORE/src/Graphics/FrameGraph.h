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
		class Node
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
				miru::Ref<objects::Camera>					camera;
				bool										cameraForce;
				miru::Ref<objects::Camera>					fontCamera;
				bool										fontCameraForce;
				miru::Ref<objects::Skybox>					skybox;
				bool										skyboxForce;
				std::vector<miru::Ref<objects::Light>>		lights;
				bool										lightsForce;
				std::vector<miru::Ref<objects::Model>>		models;
				bool										modelsForce;
				bool										materialsForce;
			};
			struct TransitionResourcesTaskInfo
			{
				miru::crossplatform::PipelineStageBit					srcPipelineStage;
				miru::crossplatform::PipelineStageBit					dstPipelineStage;
				std::vector<miru::Ref<miru::crossplatform::Barrier>>	barriers;
			};

			struct CreateInfo
			{
				std::string											debugName;
				Task												task;
				void*												pTaskInfo;
				std::vector<gear::Ref<Node>>						srcNodes;
				std::vector<miru::crossplatform::PipelineStageBit>	srcPipelineStages;
				miru::Ref<miru::crossplatform::CommandBuffer>		cmdBuffer;
				uint32_t											cmdBufferIndex;
				bool												resetCmdBuffer;
				bool												submitCmdBuffer;
				bool												skipTask;
			};

		private:
			CreateInfo m_CI;

			miru::Ref<miru::crossplatform::Fence> m_CmdBufferFence;
			miru::crossplatform::Fence::CreateInfo m_CmdBufferFenceCI;

			miru::Ref<miru::crossplatform::Semaphore> m_SignalSemaphore;
			miru::crossplatform::Semaphore::CreateInfo m_SignalSemaphoreCI;


		public:
			Node(CreateInfo* pCreateInfo);
			~Node();

			void Execute();

			inline const CreateInfo& GetCreateInfo() const { return m_CI; }

			inline const miru::Ref<miru::crossplatform::Fence>& GetFence() const { return m_CmdBufferFence; }
			inline const miru::Ref<miru::crossplatform::Semaphore>& GetSemaphore() const { return m_SignalSemaphore; }

		private:
			std::vector<std::pair<miru::crossplatform::PipelineStageBit, gear::Ref<Node>>> LastSubmitNodes(
				std::vector<miru::crossplatform::PipelineStageBit>& srcPipelineStages, std::vector<gear::Ref<Node>>& srcNodes);
			void TransitionResources();
			void UploadResources();

		};
	}
}
