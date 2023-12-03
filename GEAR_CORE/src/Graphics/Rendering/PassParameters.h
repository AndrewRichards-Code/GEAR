#pragma once
#include "Graphics/Rendering/Resource.h"

namespace gear
{
	namespace graphics
	{
		class Vertexbuffer;
		class Indexbuffer;
		class RenderPipeline;
	}
	namespace graphics::rendering
	{
		class Pass;
		class RenderGraph;

		class GEAR_API PassParameters
		{
			//structs/enums
		protected:
			enum class Type : uint32_t { TASK, TRANSFER };

			//Methods
		protected:
			virtual ~PassParameters();

			virtual void Setup() = 0;

			inline std::vector<ResourceView>& GetInputResourceViews() { return m_InputResourceViews; }
			inline std::vector<ResourceView>& GetOutputResourceViews() { return m_OutputResourceViews; }

			inline const Type& GetType() { return m_Type; }

			//Members
		protected:
			Type m_Type;

			std::vector<ResourceView> m_InputResourceViews;
			std::vector<ResourceView> m_OutputResourceViews;

			//Friends
			friend Pass;
			friend RenderGraph;
		};

		class GEAR_API TaskPassParameters final : public PassParameters
		{
			//Methods
		public:
			TaskPassParameters(const Ref<RenderPipeline>&pipeline);
			~TaskPassParameters();

		private:
			void Setup() override;

		public:
			const std::pair<uint32_t, uint32_t> FindResourceViewSetBinding(const std::string& name) const;
			void SetResourceView(const std::pair<uint32_t, uint32_t>& set_binding, ResourceView resourceView);
			void SetResourceView(const std::string& name, ResourceView resourceView);

			void AddVertexBuffer(ResourceView resourceView);
			void AddIndexBuffer(ResourceView resourceView);

			void AddAttachment(uint32_t index, const ResourceView& resource, miru::base::RenderPass::AttachmentLoadOp loadOp, miru::base::RenderPass::AttachmentStoreOp storeOp, const miru::base::Image::ClearValue& clearValue);  //DepthStencil attachments ignore the 'index' parameter.
			void AddAttachmentWithResolve(uint32_t index, const ResourceView& resource, const ResourceView& resolveResourceView, miru::base::RenderPass::AttachmentLoadOp loadOp, miru::base::RenderPass::AttachmentStoreOp storeOp, const miru::base::Image::ClearValue& clearValue); //DepthStencil attachments ignore the 'index' parameter.
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

			inline const miru::base::PipelineRef& GetPipeline() const;
			inline const std::vector<miru::base::DescriptorSetRef> GetDescriptorSets() const
			{
				std::vector<miru::base::DescriptorSetRef> result;
				for (const auto& descriptorSet : m_DescriptorSets)
					result.push_back(descriptorSet.second);
				return result;
			}
			inline const miru::base::RenderingInfo& GetRenderingInfo() const { return m_RenderingInfo; }

			inline bool IsGraphics() { return (m_RenderingInfo.colourAttachments.size() > 0 || m_RenderingInfo.pDepthAttachment != nullptr); }
			inline bool IsCompute() { return IsGraphics(); }

			//Members
		private:
			Ref<RenderPipeline> m_RenderPipeline;

			miru::base::DescriptorPoolRef m_DescriptorPool;
			std::map<uint32_t, miru::base::DescriptorSetRef> m_DescriptorSets;

			miru::base::RenderingAttachmentInfo m_DepthAttachmentInfo;
			miru::base::RenderingInfo m_RenderingInfo;

			//Friends
			friend Pass;
			friend RenderGraph;
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
			struct ResourcePairs
			{
				ResourceView		src;
				ResourceView		dst;
				ResourceCopyRegion	rcr;
			};
			//Methods
		public:
			TransferPassParameters();
			~TransferPassParameters();

		private:
			void Setup() override {};

		public:
			void AddResourceView(const Ref<Vertexbuffer>& vertexbuffer);
			void AddResourceView(const Ref<Indexbuffer>& indexbuffer);
			void AddResourceView(const Ref<BaseUniformbuffer>& uniformbuffer);
			void AddResourceView(const Ref<BaseStoragebuffer>& storagebuffer);
			void AddResourceView(const Ref<Texture>& texture);
			void AddResourceViewPair(const ResourceView& srcResourceView, const ResourceView& dstResourceView, const ResourceCopyRegion& copyRegion);

			inline const std::vector<ResourcePairs>& GetResourceViewsPairs() const { return m_ResourceViewPairs; }
			inline std::vector<ResourcePairs>& GetResourceViewsPairs() { return m_ResourceViewPairs; }

			//Members
		private:
			std::vector<ResourcePairs> m_ResourceViewPairs;

			//Friends
			friend Pass;
			friend RenderGraph;
		};
	}
}