#pragma once
#include "Graphics/Rendering/PassParameters.h"

namespace gear
{
	namespace graphics::rendering
	{
		typedef std::function<void(miru::base::CommandBufferRef&, uint32_t frameIndex)> RenderGraphPassFunction;

		class GEAR_API Pass
		{
			//Methods
		public:
			Pass(const std::string& passName, const Ref<PassParameters>& passParameters, miru::base::CommandPool::QueueType queueType, RenderGraphPassFunction renderFunction);
			Pass(const Pass&) = delete;
			~Pass();

			void Execute(RenderGraph* renderGraph, miru::base::CommandBufferRef cmdBuffer, uint32_t frameIndex);

		private:
			miru::base::Barrier2Ref TransitionResource(RenderGraph* renderGraph, Resource& passResource, Resource::State overideNewState =  Resource::State::UNKNOWN);

		public:
			inline const std::string& GetName() const { return m_PassName; }

			inline Ref<PassParameters>& GetPassParameters() { return m_PassParameters; }
			inline const Ref<PassParameters>& GetPassParameters() const { return m_PassParameters; }

			inline std::vector<Resource>& GetInputResources() { return m_PassParameters->GetInputResources(); }
			inline std::vector<Resource>& GetOutputResources() { return m_PassParameters->GetOutputResources(); }

			//Members
		private:
			std::string m_PassName;
			Ref<PassParameters> m_PassParameters;
			miru::base::CommandPool::QueueType m_QueueType;
			RenderGraphPassFunction m_RenderFunction;

			std::vector<Ref<Pass>> m_BackwardGraphicsDependentPasses;
			std::vector<Ref<Pass>> m_ForwardGraphicsDependentPasses;

			size_t m_UnorderedListIndex = 0;
			size_t m_DependencyLevelIndex = 0;

			//Friends
			friend RenderGraph;
		};
	}
}