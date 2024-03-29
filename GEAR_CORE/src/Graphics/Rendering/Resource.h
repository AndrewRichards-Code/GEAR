#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace graphics
	{
		class Texture;
		class Vertexbuffer;
		class Indexbuffer;
		class BaseUniformbuffer;
		class BaseStoragebuffer;
	}
	namespace graphics::rendering
	{
		class GEAR_API Resource
		{
			//structs/enums
		public:
			enum class Type : uint32_t
			{
				IMAGE,
				SAMPLER,
				BUFFER,
				ACCELERATION_STRUCTURE
			};
			enum class State : uint32_t
			{
				UNKNOWN,
				PRESENT,
				COLOUR_ATTACHMENT,
				DEPTH_STENCIL_ATTACHMENT,
				VERTEX_BUFFER,
				INDEX_BUFFER,
				UNIFORM_BUFFER,
				SHADER_READ_ONLY,
				SHADER_READ_WRITE,
				TRANSFER_SRC,
				TRANSFER_DST,
			};

			//Methods
		public:
			Resource();
			~Resource();

			Resource(const miru::base::ImageRef& image);
			Resource(const miru::base::SamplerRef& sampler);
			Resource(const miru::base::BufferRef& buffer);
			Resource(const miru::base::AccelerationStructureRef& accelerationStructure);

			bool operator== (const Resource& other) const;
			bool operator!= (const Resource& other) const;
			
			const std::string& GetName() const;
			std::string& GetName();

			const Type& GetType() const;
			const Type& GetType();

			State GetSubresources(uint32_t mipLevel = 0, uint32_t arrayLayer = 0);
			State GetSubresources(const miru::base::Image::SubresourceRange& subresourceRange);

			void SetSubresources(State state);
			void SetSubresources(State state, const miru::base::Image::SubresourceRange& subresourceRange);

			bool AreSubresourcesInSameState(const miru::base::Image::SubresourceRange& subresourceRange);
			std::vector<std::pair<uint32_t, uint32_t>> GetSubresourcesToTransition(State state, const miru::base::Image::SubresourceRange& subresourceRange);

		private:
			template<typename T>
			void Initialise(const T& internalResource);

			//Members
		private:
			std::string								name;
			Type									type;
			miru::base::ImageRef					image;
			miru::base::SamplerRef					sampler;
			miru::base::BufferRef					buffer;
			miru::base::AccelerationStructureRef	accelerationStructure;

			//subresourceMap[mipLevel][arrayLayer]
			std::map<uint32_t, std::map<uint32_t, State>> subresourceMap;

			//Friends
			friend class TaskPassParameters;
			friend class TransferPassParameters;
			friend class Pass;
			friend class RenderGraph;
		};

		class ResourceView
		{
			//Methods
		public:
			ResourceView();
			~ResourceView();

			ResourceView(const Ref<Texture>& texture, miru::base::DescriptorType descType); //For Shader Resorces
			ResourceView(const Ref<Texture>& texture, Resource::State state); //For Rendering Attachments
			ResourceView(const Ref<Vertexbuffer>& vertexBuffer);
			ResourceView(const Ref<Indexbuffer>& indexBuffer);
			ResourceView(const Ref<BaseUniformbuffer>& uniformBuffer);
			ResourceView(const Ref<BaseStoragebuffer>& storageBuffer);

			ResourceView(const miru::base::ImageViewRef& imageView, const miru::base::SamplerRef& sampler);
			ResourceView(const miru::base::ImageViewRef& imageView, Resource::State state);
			ResourceView(const miru::base::SamplerRef& sampler);
			ResourceView(const miru::base::BufferViewRef& bufferView, Resource::State state);
			ResourceView(const miru::base::AccelerationStructureRef& accelerationStructure);

			Resource GetResource() const;
			Resource GetResource();

			bool operator== (const ResourceView& other) const;
			bool operator!= (const ResourceView& other) const;

			inline bool IsImageView() const { return imageView != nullptr; }
			inline bool IsSampler() const { return sampler != nullptr; }
			inline bool IsBufferView() const { return bufferView != nullptr; }
			inline bool IsAccelerationStructure() const { return accelerationStructure != nullptr; }

			inline const Resource::State& GetState() const { return state; }
			inline const Resource::State& GetState() { return state; }

			inline const miru::base::PipelineStageBit& GetPipelineStageBit() const { return pipelineStage; }
			inline const miru::base::PipelineStageBit& GetPipelineStageBit() { return pipelineStage; }

			static miru::base::PipelineStageBit ShaderStageToPipelineStage(const miru::base::Shader::StageBit& stage);
			
			//Members
		private:
			static constexpr miru::base::DescriptorType	NonDescriptorType = miru::base::DescriptorType(~0);
			
			miru::base::DescriptorType				descriptorType = NonDescriptorType;
			miru::base::PipelineStageBit			pipelineStage = miru::base::PipelineStageBit::NONE_BIT; //Not set in constructors.
			Resource::State							state = Resource::State::UNKNOWN;

			miru::base::ImageViewRef				imageView;
			miru::base::SamplerRef					sampler;
			miru::base::BufferViewRef				bufferView;
			miru::base::AccelerationStructureRef	accelerationStructure;

			//Friends
			friend class TaskPassParameters;
			friend class TransferPassParameters;
			friend class Pass;
			friend class RenderGraph;
		};
	}
}