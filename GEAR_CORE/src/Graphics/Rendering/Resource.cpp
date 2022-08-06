#include "gear_core_common.h"
#include "Graphics/Rendering/Resource.h"

#include "Graphics/IndexBuffer.h"
#include "Graphics/Storagebuffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Uniformbuffer.h"
#include "Graphics/VertexBuffer.h"

using namespace gear;
using namespace graphics;
using namespace rendering;

using namespace miru;
using namespace base;

////////////
//Resource//
////////////

Resource::Resource()
{
}

Resource::~Resource()
{
}

Resource::Resource(const Ref<Texture>& texture, DescriptorType _type)
{
	type = _type;
	switch (type)
	{
	case DescriptorType::SAMPLER:
		newState = State::SHADER_READ_ONLY;
		sampler = texture->GetSampler();
		break;
	case DescriptorType::COMBINED_IMAGE_SAMPLER:
		newState = State::SHADER_READ_ONLY;
		sampler = texture->GetSampler();
		imageView = texture->GetImageView();
		break;
	case DescriptorType::SAMPLED_IMAGE:
		newState = State::SHADER_READ_ONLY;
		imageView = texture->GetImageView();
		break;
	case DescriptorType::STORAGE_IMAGE:
		newState = State::SHADER_READ_WRITE;
		imageView = texture->GetImageView();
		break;
	case DescriptorType::INPUT_ATTACHMENT:
		newState = State::SHADER_READ_ONLY;
		imageView = texture->GetImageView();
		break;
	default:
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "DescriptorType is invalid for a Texture Resource");
	}
}

Resource::Resource(const Ref<Texture>& texture, State _state)
{
	newState = _state;
	type = _state == State::SHADER_READ_ONLY ? DescriptorType::COMBINED_IMAGE_SAMPLER : _state == State::SHADER_READ_WRITE ? DescriptorType::STORAGE_IMAGE : DescriptorType(-1);
	imageView = texture->GetImageView();

	if (_state == State::SHADER_READ_ONLY)
		sampler = texture->GetSampler();
}

Resource::Resource(const Ref<BaseUniformbuffer>& uniformBuffer)
{
	type = DescriptorType::UNIFORM_BUFFER;
	bufferView = uniformBuffer->GetGPUBufferView();
}

Resource::Resource(const Ref<BaseStoragebuffer>& storageBuffer)
{
	type = DescriptorType::STORAGE_BUFFER;
	bufferView = storageBuffer->GetGPUBufferView();
}

Resource::Resource(const ImageViewRef& _imageView, const SamplerRef& _sampler)
{
	newState = State::SHADER_READ_ONLY;
	type = DescriptorType::COMBINED_IMAGE_SAMPLER;
	imageView = _imageView;
	sampler = _sampler;
}

Resource::Resource(const ImageViewRef& _imageView, State _state)
{
	newState = _state;
	type = _state == State::SHADER_READ_ONLY ? DescriptorType::SAMPLED_IMAGE : _state == State::SHADER_READ_WRITE ? DescriptorType::STORAGE_IMAGE : NonDescriptorType;
	imageView = _imageView;
}

Resource::Resource(const SamplerRef& _sampler)
{
	newState = State::SHADER_READ_ONLY;
	type = DescriptorType::SAMPLER;
	sampler = _sampler;
}

Resource::Resource(const BufferViewRef& _bufferView, State _state)
{
	newState = _state;
	type = _state == State::SHADER_READ_ONLY ? DescriptorType::UNIFORM_BUFFER : _state == State::SHADER_READ_WRITE ? DescriptorType::STORAGE_BUFFER : NonDescriptorType;
	bufferView = _bufferView;
}

Resource::Resource(const AccelerationStructureRef& _accelerationStructure)
{
	newState = State::SHADER_READ_ONLY;
	type = DescriptorType::ACCELERATION_STRUCTURE;
	accelerationStructure = _accelerationStructure;
}

const std::string& gear::graphics::rendering::Resource::GetName() const
{
	if (IsImageView())
		return imageView->GetCreateInfo().debugName;
	else if (IsSampler())
		return sampler->GetCreateInfo().debugName;
	else if (IsBufferView())
		return bufferView->GetCreateInfo().debugName;
	else if (IsAccelerationStructure())
		return accelerationStructure->GetCreateInfo().debugName;
	else
		return std::string("");
}

bool Resource::operator== (const Resource& other) const
{
	if (IsImageView() && other.IsImageView())
	{
		if (imageView == other.imageView)
			return true;
		else 
			return imageView->GetCreateInfo().image == other.imageView->GetCreateInfo().image;
	}
	else if (IsSampler() && other.IsSampler())
		return sampler == other.sampler;
	else if (IsBufferView() && other.IsBufferView())
	{
		if (bufferView == other.bufferView)
			return true;
		else
			return bufferView->GetCreateInfo().buffer == other.bufferView->GetCreateInfo().buffer;
	}
	else if (IsAccelerationStructure() && other.IsAccelerationStructure())
		return accelerationStructure == other.accelerationStructure;
	else
		return false;
}

bool Resource::operator!= (const Resource& other) const
{
	return !(*this == other);
}