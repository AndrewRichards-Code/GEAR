#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace graphics
	{
		class Texture;
		class BaseUniformbuffer;
		class BaseStoragebuffer;
	}
	namespace graphics::rendering
	{
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

			const std::string& GetName() const;

			bool operator== (const Resource& other) const;
			bool operator!= (const Resource& other) const;

			inline bool IsImageView() const { return imageView != nullptr; }
			inline bool IsSampler() const { return sampler != nullptr; }
			inline bool IsBufferView() const { return bufferView != nullptr; }
			inline bool IsAccelerationStructure() const { return accelerationStructure != nullptr; }

			const State& GetOldState() const { return oldState; }
			const State& GetNewState() const { return newState; }
			const miru::base::Shader::StageBit& GetStage() const { return stage; }

			static constexpr miru::base::DescriptorType NonDescriptorType = miru::base::DescriptorType(-1);

			//Members
		private:
			State									oldState = State::UNKNOWN;
			State									newState;
			miru::base::Shader::StageBit			stage = miru::base::Shader::StageBit(0);
			miru::base::DescriptorType				type;
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