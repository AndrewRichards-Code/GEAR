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
				miru::base::CommandPoolRef cmdPool;
				miru::base::CommandPool::CreateInfo cmdPoolCI;
				miru::base::CommandBufferRef cmdBuffer;
				miru::base::CommandBuffer::CreateInfo cmdBufferCI;
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
			struct DependencyLevel
			{
				std::vector<Ref<Pass>> m_Passes;
				size_t m_LevelIndex = 0;
			};

			//Methods
		public:
			RenderGraph(const miru::base::ContextRef& context, uint32_t commandBufferCount);
			~RenderGraph();

			Ref<Pass> AddPass(const std::string& passName, const Ref<PassParameters>& passParameters, miru::base::CommandPool::QueueType queueType, RenderGraphPassFunction renderFunction);

		private:
			void Compile();

		public:
			void Execute(uint32_t frameIndex);
			void Reset();

			miru::base::ImageRef CreateImage(const ImageDesc& desc, const std::string& name);
			miru::base::ImageViewRef CreateImageView(const miru::base::ImageRef& image, miru::base::Image::Type type, const miru::base::Image::SubresourceRange& subresourceRange);

			inline const std::vector<Ref<Pass>>& GetPasses() const { return m_Passes; }
			inline const std::vector<Ref<Pass>>& GetTopologicallySortedPasses() const { return m_TopologicallySortedPasses; }

			Resource& GetTrackedResource(const Resource& passResource);
			const Resource& GetTrackedResource(const Resource& passResource) const;
			miru::base::CommandBufferRef& GetCommandBuffer(miru::base::CommandPool::QueueType queueType);

			//Members
		private:
			miru::base::ContextRef m_Context;
			std::map<miru::base::CommandPool::QueueType, CommandPoolAndBuffers> m_CommandPoolAndBuffers;

			std::vector<Ref<Pass>> m_Passes;
			std::vector<Resource> m_Resources;
			std::vector<Resource> m_PreviousFrameResources;
			
			std::vector<std::vector<size_t>> m_AdjacencyLists;
			std::vector<Ref<Pass>> m_TopologicallySortedPasses;
			std::vector<DependencyLevel> m_DependencyLevels;
		};
	}
}
