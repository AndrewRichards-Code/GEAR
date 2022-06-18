#pragma once
#include "gear_core_common.h"
#include "Graphics/Texture.h"
#include "Graphics/Uniformbuffer.h"
#include "Graphics/Storagebuffer.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"

namespace gear
{
	namespace graphics
	{
		class PassParameters;
		class RenderPassParameters;
		class TransferPassParameters;
		class Pass;
		class RenderGraph;
		typedef std::function<void(miru::base::CommandBufferRef&, uint32_t frameIndex)> RenderGraphPassFunction;

		class GEAR_API Resource
		{
			//structs/enums
		public:
			enum class State :uint32_t
			{
				UNKNOWN,
				PRESENT,
				COLOUR_ATTACHMENT,
				DEPTH_STENCIL_ATTACHMENT,
				SHADER_READ_ONLY,
				SHADER_READ_WRITE,
				TRANSFER_SRC,
				TRANSFER_DST,
			};

			//Methods
		public:
			Resource();
			~Resource();

			//For Shader Resorces
			Resource(const Ref<Texture>& texture, miru::base::DescriptorType _type);
			//For Rendering Attachments
			Resource(const Ref<Texture>& texture, State _state);
			Resource(const Ref<BaseUniformbuffer>& uniformBuffer);
			Resource(const Ref<BaseStoragebuffer>& storageBuffer);

			Resource(const miru::base::ImageViewRef& _imageView, const miru::base::SamplerRef& _sampler);
			Resource(const miru::base::ImageViewRef& _imageView, State _state);
			Resource(const miru::base::SamplerRef& _sampler);
			Resource(const miru::base::BufferViewRef& _bufferView, State _state);
			Resource(const miru::base::AccelerationStructureRef& _accelerationStructure);

			bool operator== (const Resource& other) const;
			bool operator!= (const Resource& other) const;

			inline bool IsImageView() const { return imageView != nullptr; }
			inline bool IsSampler() const { return sampler != nullptr; }
			inline bool IsBufferView() const { return bufferView != nullptr; }
			inline bool IsAccelerationStructure() const { return accelerationStructure != nullptr; }

			static constexpr miru::base::DescriptorType NonDescriptorType = miru::base::DescriptorType(-1);

			//Members
		private:
			State									oldState = State::UNKNOWN;
			State									newState;
			miru::base::DescriptorType				type;
			miru::base::ImageViewRef				imageView;
			miru::base::SamplerRef					sampler;
			miru::base::BufferViewRef				bufferView;
			miru::base::AccelerationStructureRef	accelerationStructure;

			//Friends
			friend RenderPassParameters;
			friend TransferPassParameters;
			friend Pass;
			friend RenderGraph;
		};

		class GEAR_API PassParameters
		{
			//structs/enums
		protected:
			enum class Type : uint32_t { RENDER, TRANSFER };

			//Methods
		protected:
			virtual ~PassParameters();

			virtual void Setup() = 0;

			inline std::vector<Resource>& GetInputResources() { return m_InputResources; }
			inline std::vector<Resource>& GetOutputResources() { return m_OutputResources; }

			inline const Type& GetType() { return m_Type; }

			//Members
		protected:
			Type m_Type;

			std::vector<Resource> m_InputResources;
			std::vector<Resource> m_OutputResources;

			//Friends
			friend Pass;
			friend RenderGraph;
		};

		class GEAR_API RenderPassParameters final : public PassParameters
		{
			//Methods
		public:
			RenderPassParameters(const Ref<RenderPipeline>& pipeline, const std::vector<size_t>& setMultiplers = {});
			~RenderPassParameters();

		private:
			void Setup() override;

		public:
			const std::pair<uint32_t, uint32_t> FindResourceSetBinding(const std::string& name) const;
			void SetResource(const std::pair<uint32_t, uint32_t>& set_binding, const Resource& resource, uint32_t setIndex = 0);
			void SetResource(const std::string& name, const Resource& resource, uint32_t setIndexOffset = 0);

			void AddAttachment(uint32_t index, const Resource& resource, miru::base::RenderPass::AttachmentLoadOp loadOp, miru::base::RenderPass::AttachmentStoreOp storeOp, const miru::base::Image::ClearValue& clearValue);  //DepthStencil attachments ignore the 'index' parameter.
			void AddAttachmentWithResolve(uint32_t index, const Resource& resource, const Resource& resolveResource, miru::base::RenderPass::AttachmentLoadOp loadOp, miru::base::RenderPass::AttachmentStoreOp storeOp, const miru::base::Image::ClearValue& clearValue); //DepthStencil attachments ignore the 'index' parameter.
			void SetRenderArea(miru::base::Rect2D renderArea, uint32_t layers = 1, uint32_t viewMask = 0);

			template<typename T>
			static inline miru::base::Viewport CreateViewport(const T& width, const T& height)
			{
				return { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
			}
			template<typename T>
			static inline miru::base::Rect2D CreateScissor(const T& width, const T& height)
			{
				return { { 0, 0 }, { static_cast<uint32_t>(width), static_cast<uint32_t>(height) } };
			}

			inline const miru::base::PipelineRef& GetPipeline() const { return m_RenderPipeline->GetPipeline(); }
			inline const miru::base::DescriptorSetRef& GetDescriptorSet(uint32_t set, uint32_t setIndex = 0) const { return m_DescriptorSets.at(set).at(setIndex); }
			inline const miru::base::RenderingInfo& GetRenderingInfo() const { return m_RenderingInfo; }

			//Members
		private:
			Ref<RenderPipeline> m_RenderPipeline;

			miru::base::DescriptorPoolRef m_DescriptorPool;
			std::map<uint32_t, std::map<uint32_t, miru::base::DescriptorSetRef>> m_DescriptorSets;

			miru::base::RenderingAttachmentInfo m_DepthAttachmentInfo;
			miru::base::RenderingInfo m_RenderingInfo;
		};

		class GEAR_API TransferPassParameters final : public PassParameters
		{
			//struct/enum
		public:
			struct ResourceCopyRegion
			{
				union
				{
					miru::base::Buffer::Copy			bufferCopy;
					miru::base::Image::BufferImageCopy	bufferImageCopy;
					miru::base::Image::Copy				imageCopy;
				};
			};
			//Methods
		public:
			TransferPassParameters();
			~TransferPassParameters();
		
		private:
			void Setup() override {};

		public:
			void AddResource(const Ref<Vertexbuffer>& vertexbuffer);
			void AddResource(const Ref<Indexbuffer>& indexbuffer);
			void AddResource(const Ref<BaseUniformbuffer>& uniformbuffer);
			void AddResource(const Ref<BaseStoragebuffer>& storagebuffer);
			void AddResource(const Ref<Texture>& texture);
			void AddResourcePair(const Resource& srcResource, const Resource& dstResource, const ResourceCopyRegion copyRegion);

			inline const std::vector<std::tuple<Resource, Resource, ResourceCopyRegion>>& GetResourcesPairs() const { return m_ResourcePairs; }

			//Members
		private:
			std::vector<std::tuple<Resource, Resource, ResourceCopyRegion>> m_ResourcePairs;
		};

		class GEAR_API Pass
		{
			//Methods
		public:
			Pass(const std::string& passName, const Ref<PassParameters>& passParameters, miru::base::CommandPool::QueueType queueType, RenderGraphPassFunction renderFunction);
			Pass(const Pass&) = delete;
			~Pass();

			void Execute(miru::base::CommandBufferRef cmdBuffer, uint32_t frameIndex);

		private:
			miru::base::BarrierRef TransitionResource(Resource& resource);

		public:
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

			miru::base::CommandBufferRef& GetCommandBuffer(miru::base::CommandPool::QueueType queueType);

			//Members
		private:
			miru::base::ContextRef m_Context;
			std::map<miru::base::CommandPool::QueueType, CommandPoolAndBuffers> m_CommandPoolAndBuffers;

			std::vector<Ref<Pass>> m_Passes;
			std::vector<Resource> m_Resources;
			
			std::vector<std::vector<size_t>> m_AdjacencyLists;
			std::vector<Ref<Pass>> m_TopologicallySortedPasses;
			std::vector<DependencyLevel> m_DependencyLevels;
		};

		
	}
}
