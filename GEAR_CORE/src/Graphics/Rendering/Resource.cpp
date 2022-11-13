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

Resource::Resource(const ImageRef& image)
{
	this->image = image;
	Init(image);
}

Resource::Resource(const SamplerRef& sampler)
{
	this->sampler = sampler;
	Init(sampler);
}

Resource::Resource(const BufferRef& buffer)
{
	this->buffer = buffer;
	Init(buffer);
}

Resource::Resource(const AccelerationStructureRef& accelerationStructure)
{
	this->accelerationStructure = accelerationStructure;
	Init(accelerationStructure);
}

template<typename T>
void Resource::Init(const T& internalResource)
{
	if (typeid(T) == typeid(miru::base::ImageRef))
	{
		type = Type::IMAGE;
		name = image->GetCreateInfo().debugName;
		for (uint32_t level = 0; level < image->GetCreateInfo().mipLevels; level++)
			for (uint32_t layer = 0; layer < image->GetCreateInfo().arrayLayers; layer++)
				subresourceMap[level][layer] = State::UNKNOWN;
	}
	else if (typeid(T) == typeid(miru::base::SamplerRef))
	{
		type = Type::SAMPLER;
		name = sampler->GetCreateInfo().debugName;
		subresourceMap[0][0] = State::UNKNOWN;
	}
	else if (typeid(T) == typeid(miru::base::BufferRef))
	{
		type = Type::BUFFER;
		name = buffer->GetCreateInfo().debugName;
		subresourceMap[0][0] = State::UNKNOWN;
	}
	else if (typeid(T) == typeid(miru::base::AccelerationStructureRef))
	{
		type = Type::ACCELERATION_STRUCTURE;
		name = accelerationStructure->GetCreateInfo().debugName;
		subresourceMap[0][0] = State::UNKNOWN;
	}
	else
	{
		auto _break = []()
		{
			GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Unknown internal resource type.");
		};
		_break();
	}

}

bool Resource::operator== (const Resource& other) const
{
	return (type == other.type
		&& image == other.image
		&& sampler == other.sampler
		&& buffer == other.buffer
		&& accelerationStructure == other.accelerationStructure);
}

bool Resource::operator!= (const Resource& other) const
{
	return !(*this == other);
}

const std::string& Resource::GetName() const
{ 
	return name;
}
std::string& Resource::GetName()
{ 
	return name;
}

const Resource::Type& Resource::GetType() const
{ 
	return type;
}
const Resource::Type& Resource::GetType()
{ 
	return type;
}

Resource::State Resource::GetSubresources(uint32_t mipLevel, uint32_t arrayLayer)
{
	return subresourceMap[mipLevel][arrayLayer];
}

Resource::State Resource::GetSubresources(const miru::base::Image::SubresourceRange& subresourceRange)
{
	bool same = AreSubresourcesInSameState(subresourceRange);
	if (!same)
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_STATE, "Not all subresources the range have the same State.");
		return State::UNKNOWN;
	}
	else
	{
		return subresourceMap[subresourceRange.baseMipLevel][subresourceRange.baseArrayLayer];
	}

}

void Resource::SetSubresources(State state)
{
	subresourceMap[0][0] = state;
}

void Resource::SetSubresources(State state, const miru::base::Image::SubresourceRange& subresourceRange)
{
	for (uint32_t level = subresourceRange.baseMipLevel; level < subresourceRange.baseMipLevel + subresourceRange.mipLevelCount; level++)
		for (uint32_t layer = subresourceRange.baseArrayLayer; layer < subresourceRange.baseArrayLayer + subresourceRange.arrayLayerCount; layer++)
			subresourceMap[level][layer] = state;
}

bool Resource::AreSubresourcesInSameState(const miru::base::Image::SubresourceRange& subresourceRange)
{
	std::vector<State> stateCheck;
	for (uint32_t level = subresourceRange.baseMipLevel; level < subresourceRange.baseMipLevel + subresourceRange.mipLevelCount; level++)
		for (uint32_t layer = subresourceRange.baseArrayLayer; layer < subresourceRange.baseArrayLayer + subresourceRange.arrayLayerCount; layer++)
			stateCheck.push_back(subresourceMap[level][layer]);

	if (stateCheck.empty())
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_STATE, "Invalid Image::SubresourceRange.");
	}

	return std::equal(stateCheck.begin() + 1, stateCheck.end(), stateCheck.begin());
}

std::vector<std::pair<uint32_t, uint32_t>> Resource::GetSubresourcesToTransition(State state, const miru::base::Image::SubresourceRange& subresourceRange)
{
	std::vector<std::pair<uint32_t, uint32_t>> result;
	for(uint32_t level = subresourceRange.baseMipLevel; level < subresourceRange.baseMipLevel + subresourceRange.mipLevelCount; level++)
	{
		for (uint32_t layer = subresourceRange.baseArrayLayer; layer < subresourceRange.baseArrayLayer + subresourceRange.arrayLayerCount; layer++)
		{
			if (state != GetSubresources(level, layer))
				result.push_back({ level, layer });
		}
	}
	return result;
}

////////////////
//ResourceView//
////////////////

ResourceView::ResourceView()
{
}

ResourceView::~ResourceView()
{
}

ResourceView::ResourceView(const Ref<Texture>& texture, miru::base::DescriptorType type)
{
	this->type = type;
	switch (type)
	{
	case DescriptorType::SAMPLER:
		state = Resource::State::SHADER_READ_ONLY;
		sampler = texture->GetSampler();
		break;
	case DescriptorType::COMBINED_IMAGE_SAMPLER:
		state = Resource::State::SHADER_READ_ONLY;
		sampler = texture->GetSampler();
		imageView = texture->GetImageView();
		break;
	case DescriptorType::SAMPLED_IMAGE:
		state = Resource::State::SHADER_READ_ONLY;
		imageView = texture->GetImageView();
		break;
	case DescriptorType::STORAGE_IMAGE:
		state = Resource::State::SHADER_READ_WRITE;
		imageView = texture->GetImageView();
		break;
	case DescriptorType::INPUT_ATTACHMENT:
		state = Resource::State::SHADER_READ_ONLY;
		imageView = texture->GetImageView();
		break;
	default:
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "DescriptorType is invalid for a Texture Resource");
	}
}

ResourceView::ResourceView(const Ref<Texture>& texture, Resource::State state)
{
	this->state = state;
	type = state == Resource::State::SHADER_READ_ONLY ? DescriptorType::COMBINED_IMAGE_SAMPLER : state == Resource::State::SHADER_READ_WRITE ? DescriptorType::STORAGE_IMAGE : NonDescriptorType;
	imageView = texture->GetImageView();

	if (state == Resource::State::SHADER_READ_ONLY)
		sampler = texture->GetSampler();
} 

ResourceView::ResourceView(const Ref<Vertexbuffer>& vertexBuffer)
{
	state = Resource::State::VERTEX_BUFFER;
	type = NonDescriptorType;
	bufferView = vertexBuffer->GetGPUBufferView();
}

ResourceView::ResourceView(const Ref<Indexbuffer>& indexBuffer)
{
	state = Resource::State::INDEX_BUFFER;
	type = NonDescriptorType;
	bufferView = indexBuffer->GetGPUBufferView();
}

ResourceView::ResourceView(const Ref<BaseUniformbuffer>& uniformBuffer)
{
	type = DescriptorType::UNIFORM_BUFFER;
	bufferView = uniformBuffer->GetGPUBufferView();
}

ResourceView::ResourceView(const Ref<BaseStoragebuffer>& storageBuffer)
{
	type = DescriptorType::STORAGE_BUFFER;
	bufferView = storageBuffer->GetGPUBufferView();
}

ResourceView::ResourceView(const miru::base::ImageViewRef& imageView, const miru::base::SamplerRef& sampler)
{
	state = Resource::State::SHADER_READ_ONLY;
	type = DescriptorType::COMBINED_IMAGE_SAMPLER;
	this->imageView = imageView;
	this->sampler = sampler;
}

ResourceView::ResourceView(const miru::base::ImageViewRef& imageView, Resource::State state)
{
	this->state = state;
	type = state == Resource::State::SHADER_READ_ONLY ? DescriptorType::SAMPLED_IMAGE : state == Resource::State::SHADER_READ_WRITE ? DescriptorType::STORAGE_IMAGE : NonDescriptorType;
	this->imageView = imageView;
}

ResourceView::ResourceView(const miru::base::SamplerRef& sampler)
{
	state = Resource::State::SHADER_READ_ONLY;
	type = DescriptorType::SAMPLER;
	this->sampler = sampler;
}

ResourceView::ResourceView(const miru::base::BufferViewRef& bufferView, Resource::State state)
{
	this->state = state;
	type = state == Resource::State::UNIFORM_BUFFER ? DescriptorType::UNIFORM_BUFFER : state == Resource::State::SHADER_READ_WRITE ? DescriptorType::STORAGE_BUFFER : NonDescriptorType;
	this->bufferView = bufferView;
}

ResourceView::ResourceView(const miru::base::AccelerationStructureRef& accelerationStructure)
{
	this->state = Resource::State::SHADER_READ_ONLY;
	type = DescriptorType::ACCELERATION_STRUCTURE;
	this->accelerationStructure = accelerationStructure;
}

Resource ResourceView::GetResource() const
{
	if (imageView)
		return Resource(imageView->GetCreateInfo().image);
	else if (sampler)
		return Resource(sampler);
	else if (bufferView)
		return Resource(bufferView->GetCreateInfo().buffer);
	else if (accelerationStructure)
		return Resource(accelerationStructure);
	else
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_STATE, "ResourceView is invalid.");
		return Resource();
	}
}

Resource ResourceView::GetResource()
{
	if (imageView)
		return Resource(imageView->GetCreateInfo().image);
	else if (sampler)
		return Resource(sampler);
	else if (bufferView)
		return Resource(bufferView->GetCreateInfo().buffer);
	else if (accelerationStructure)
		return Resource(accelerationStructure);
	else
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_STATE, "ResourceView is invalid.");
		return Resource();
	}
}

bool ResourceView::operator== (const ResourceView& other) const
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

bool ResourceView::operator!= (const ResourceView& other) const
{
	return !(*this == other);
}