#pragma once
#include "gear_core_common.h"
#include "Graphics/Rendering/Pass.h"

namespace gear
{
	namespace graphics::rendering
	{
		class Resource;
		class PassParameters;
		class Pass;

		typedef std::function<void(miru::base::CommandBufferRef&, uint32_t frameIndex)> RenderGraphPassFunction;

		class GEAR_API RenderGraph
		{
			//structs/enums
		public:
			struct CommandPoolAndBuffers
			{
				miru::base::CommandPoolRef CmdPool;
				miru::base::CommandPool::CreateInfo CmdPoolCI;
				miru::base::CommandBufferRef CmdBuffer;
				miru::base::CommandBuffer::CreateInfo CmdBufferCI;
			};
			struct DependencyLevel
			{
				std::vector<Ref<Pass>> Passes;
				size_t LevelIndex = 0;
			};
			struct FrameData
			{
				std::vector<Ref<Pass>> Passes;
				std::vector<Resource> Resources;

				std::vector<std::vector<size_t>> AdjacencyLists;
				std::vector<Ref<Pass>> TopologicallySortedPasses;
				std::vector<DependencyLevel> DependencyLevels;

				void Clear()
				{
					Resources.clear();

					for (auto& AdjacencyList : AdjacencyLists)
						AdjacencyList.clear();
					AdjacencyLists.clear();

					TopologicallySortedPasses.clear();

					for (auto& dependencyLevel : DependencyLevels)
						dependencyLevel.Passes.clear();
					DependencyLevels.clear();
					Passes.clear();
				}
			};
			struct ImageDesc
			{
				enum class UsageBit : uint32_t
				{
					COLOUR_ATTACHMENT			= 0x00000001,
					DEPTH_STENCIL_ATTACHMENT	= 0x00000002,
					SHADER_READ_ONLY			= 0x00000004,
					SHADER_READ_WRITE			= 0x00000008,

					RTV = COLOUR_ATTACHMENT,
					DSV = DEPTH_STENCIL_ATTACHMENT,
					SRV = SHADER_READ_ONLY,
					UAV = SHADER_READ_WRITE
				};

				miru::base::Image::Type				type;
				miru::base::Image::Format			format;
				uint32_t							width;
				uint32_t							height;
				uint32_t							depth;
				uint32_t							mipLevels;
				uint32_t							arrayLayers;
				miru::base::Image::SampleCountBit	sampleCount;
				UsageBit							usage;
			};
			struct BufferDesc
			{
				uint32_t size;
			};

			//Methods
		public:
			RenderGraph();
			RenderGraph(const miru::base::ContextRef& context, uint32_t commandBufferCount);
			~RenderGraph();

			void Reset(uint32_t frameIndex);

			Ref<Pass> AddPass(const std::string& passName, const Ref<PassParameters>& passParameters, miru::base::CommandPool::QueueType queueType, RenderGraphPassFunction renderFunction);

		private:
			void Compile();

		public:
			void Execute();

			miru::base::ImageRef CreateImage(const ImageDesc& desc, const std::string& name);
			miru::base::ImageViewRef CreateImageView(const miru::base::ImageRef& image, miru::base::Image::Type type, const miru::base::Image::SubresourceRange& subresourceRange);

			inline const std::vector<Ref<Pass>>& GetPasses(uint32_t frameIndex) const { return m_FrameData[frameIndex].Passes; }
			inline const std::vector<Ref<Pass>>& GetTopologicallySortedPasses(uint32_t frameIndex) const { return m_FrameData[frameIndex].TopologicallySortedPasses; }

			Resource& GetTrackedResource(const Resource& passResource);
			const Resource& GetTrackedResource(const Resource& passResource) const;
			miru::base::CommandBufferRef& GetCommandBuffer(miru::base::CommandPool::QueueType queueType);

		private:
			FrameData& GetPreviousFrameData();

			//Members
		private:
			miru::base::ContextRef m_Context;
			std::map<miru::base::CommandPool::QueueType, CommandPoolAndBuffers> m_CommandPoolAndBuffers;
			std::vector<FrameData> m_FrameData;
			uint32_t m_FrameIndex = 0;
		};
	}
}
