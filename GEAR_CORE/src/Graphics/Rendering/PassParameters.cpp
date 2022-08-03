#include "gear_core_common.h"
#include "Graphics/Rendering/PassParameters.h"

#include "Graphics/IndexBuffer.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Storagebuffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Uniformbuffer.h"
#include "Graphics/VertexBuffer.h"

using namespace gear;
using namespace graphics;
using namespace rendering;

using namespace miru;
using namespace base;

//////////////////
//PassParameters//
//////////////////

PassParameters::~PassParameters()
{
	m_InputResources.clear();
	m_OutputResources.clear();
}

////////////////////////
//TaskPassParameters//
////////////////////////

TaskPassParameters::TaskPassParameters(const Ref<RenderPipeline>& renderPipeline)
{
	m_Type = PassParameters::Type::TASK;
	m_RenderPipeline = renderPipeline;

	const RenderPipeline::ResourceBindingDescriptions& rbds = m_RenderPipeline->GetRBDs();
	std::map<DescriptorType, uint32_t> poolSizesMap;
	uint32_t maxSets = 0;

	for (uint32_t set = 0; set < static_cast<uint32_t>(rbds.size()); set++)
	{
		maxSets++;
		for (uint32_t binding = 0; binding < static_cast<uint32_t>(rbds[set].size()); binding++)
		{
			const Shader::ResourceBindingDescription& rbd = rbds[set][binding];
			poolSizesMap[rbd.type] += rbd.descriptorCount;
		}
	}

	DescriptorPool::CreateInfo descPoolCI;
	descPoolCI.debugName = "GEAR_CORE_DescriptorPool_RenderGraph_" + m_RenderPipeline->GetCreateInfo().debugName;
	descPoolCI.device = m_RenderPipeline->GetCreateInfo().shaderCreateInfo[0].device;
	for (auto& poolSize : poolSizesMap)
		descPoolCI.poolSizes.push_back({ poolSize.first, poolSize.second });
	descPoolCI.maxSets = maxSets;
	m_DescriptorPool = DescriptorPool::Create(&descPoolCI);

	DescriptorSet::CreateInfo descSetPerViewCI;
	descSetPerViewCI.descriptorPool = m_DescriptorPool;
	for (uint32_t set = 0; set < static_cast<uint32_t>(rbds.size()); set++)
	{
		descSetPerViewCI.debugName = "GEAR_CORE_DescriptorSet_RenderGraph_" + m_RenderPipeline->GetCreateInfo().debugName + " Set: " + std::to_string(set);
		descSetPerViewCI.descriptorSetLayouts = { m_RenderPipeline->GetDescriptorSetLayouts()[set] };
		m_DescriptorSets[set] = DescriptorSet::Create(&descSetPerViewCI);
	}

	m_RenderingInfo.flags = RenderingFlagBits::NONE_BIT;
	m_RenderingInfo.colourAttachments.resize(m_RenderPipeline->GetCreateInfo().dynamicRendering.colourAttachmentFormats.size());
	m_RenderingInfo.pDepthAttachment = nullptr;
	m_RenderingInfo.pStencilAttachment = nullptr;
}

TaskPassParameters::~TaskPassParameters()
{
	m_DescriptorSets.clear();
}

void TaskPassParameters::Setup()
{
	for (auto& descSet : m_DescriptorSets)
		descSet.second->Update();
}

const std::pair<uint32_t, uint32_t> TaskPassParameters::FindResourceSetBinding(const std::string& name) const
{
	const RenderPipeline::ResourceBindingDescriptions& rbds = m_RenderPipeline->GetRBDs();
	for (uint32_t set = 0; set < static_cast<uint32_t>(rbds.size()); set++)
	{
		for (uint32_t binding = 0; binding < static_cast<uint32_t>(rbds[set].size()); binding++)
		{
			const Shader::ResourceBindingDescription& rbd = rbds[set][binding];
			bool found = false;
			if (rbd.type == DescriptorType::COMBINED_IMAGE_SAMPLER)
				found = (rbd.name.find(name) == 0);
			else
				found = (rbd.name.compare(name) == 0);
			if (found)
				return { set, binding };
		}
	}
	GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Failed to find Resource: %s in RenderPipeline: %s.", name.c_str(), m_RenderPipeline->m_CI.debugName.c_str());
	return { 0, 0 };
}

void TaskPassParameters::SetResource(const std::pair<uint32_t, uint32_t>& set_binding, const Resource& resource)
{
	const uint32_t& set = set_binding.first;
	const uint32_t& binding = set_binding.second;

	const Shader::ResourceBindingDescription& rbd = m_RenderPipeline->GetRBDs()[set][binding];
	if (rbd.type != resource.type)
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "The Resource DescriptorType does not match ResourceBindingDescription DescriptorType.");
	}

	Resource _resource = resource;
	_resource.stage = rbd.stage;

	switch (resource.type)
	{
	case DescriptorType::SAMPLER:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = resource.sampler;
		info.imageView = nullptr;
		info.imageLayout = Image::Layout(0);
		m_DescriptorSets[set]->AddImage(0, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::COMBINED_IMAGE_SAMPLER:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = resource.sampler;
		info.imageView = resource.imageView;
		info.imageLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorSets[set]->AddImage(0, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::SAMPLED_IMAGE:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = nullptr;
		info.imageView = resource.imageView;
		info.imageLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorSets[set]->AddImage(0, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::STORAGE_IMAGE:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = nullptr;
		info.imageView = resource.imageView;
		info.imageLayout = Image::Layout::GENERAL;
		m_DescriptorSets[set]->AddImage(0, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_WRITE;
		break;
	}
	case DescriptorType::UNIFORM_TEXEL_BUFFER:
	case DescriptorType::UNIFORM_BUFFER:
	case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
	{
		DescriptorSet::DescriptorBufferInfo info;
		info.bufferView = resource.bufferView;
		m_DescriptorSets[set]->AddBuffer(0, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::STORAGE_TEXEL_BUFFER:
	case DescriptorType::STORAGE_BUFFER:
	case DescriptorType::STORAGE_BUFFER_DYNAMIC:
	{
		DescriptorSet::DescriptorBufferInfo info;
		info.bufferView = resource.bufferView;
		m_DescriptorSets[set]->AddBuffer(0, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_WRITE;
		break;
	}
	case DescriptorType::INPUT_ATTACHMENT:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = nullptr;
		info.imageView = resource.imageView;
		info.imageLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorSets[set]->AddImage(0, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::ACCELERATION_STRUCTURE:
	{
		m_DescriptorSets[set]->AddAccelerationStructure(0, binding, { resource.accelerationStructure });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	}

	if (_resource.newState == Resource::State::SHADER_READ_ONLY)
	{
		m_InputResources.push_back(_resource);
	}
	else if (_resource.newState == Resource::State::SHADER_READ_WRITE)
	{
		m_InputResources.push_back(_resource);
		m_OutputResources.push_back(_resource);
	}
	else
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Resource::State is not SHADER_READ_ONLY or SHADER_READ_ONLY.");
	}
}

void TaskPassParameters::SetResource(const std::string& name, const Resource& resource)
{
	SetResource(FindResourceSetBinding(name), resource);
}

void TaskPassParameters::AddAttachment(uint32_t index, const Resource& resource, RenderPass::AttachmentLoadOp loadOp, RenderPass::AttachmentStoreOp storeOp, const Image::ClearValue& clearValue)
{
	if (resource.imageView)
	{
		if (resource.newState == Resource::State::COLOUR_ATTACHMENT)
		{
			RenderingAttachmentInfo& attachmentInfo = m_RenderingInfo.colourAttachments[index];
			attachmentInfo.imageView = resource.imageView;
			attachmentInfo.imageLayout = Image::Layout::COLOUR_ATTACHMENT_OPTIMAL;
			attachmentInfo.resolveMode = ResolveModeBits::NONE_BIT;
			attachmentInfo.resolveImageView = nullptr;
			attachmentInfo.resolveImageLayout = Image::Layout::UNKNOWN;
			attachmentInfo.loadOp = loadOp;
			attachmentInfo.storeOp = storeOp;
			attachmentInfo.clearValue = clearValue;
		}
		else if (resource.newState == Resource::State::DEPTH_STENCIL_ATTACHMENT)
		{
			RenderingAttachmentInfo& attachmentInfo = m_DepthAttachmentInfo;
			attachmentInfo.imageView = resource.imageView;
			attachmentInfo.imageLayout = Image::Layout::DEPTH_ATTACHMENT_OPTIMAL;
			attachmentInfo.resolveMode = ResolveModeBits::NONE_BIT;
			attachmentInfo.resolveImageView = nullptr;
			attachmentInfo.resolveImageLayout = Image::Layout::UNKNOWN;
			attachmentInfo.loadOp = loadOp;
			attachmentInfo.storeOp = storeOp;
			attachmentInfo.clearValue = clearValue;

			m_RenderingInfo.pDepthAttachment = &m_DepthAttachmentInfo;
		}
		else
		{
			GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Resource::State is not COLOUR_ATTACHMENT or DEPTH_STENCIL_ATTACHMENT.");
		}

		m_OutputResources.push_back(resource);
	}
}

void TaskPassParameters::AddAttachmentWithResolve(uint32_t index, const Resource& resource, const Resource& resolveResource, RenderPass::AttachmentLoadOp loadOp, RenderPass::AttachmentStoreOp storeOp, const Image::ClearValue& clearValue)
{
	if (resource.imageView && resolveResource.imageView)
	{
		if (resource.newState == Resource::State::COLOUR_ATTACHMENT)
		{
			RenderingAttachmentInfo& attachmentInfo = m_RenderingInfo.colourAttachments[index];
			attachmentInfo.imageView = resource.imageView;
			attachmentInfo.imageLayout = Image::Layout::COLOUR_ATTACHMENT_OPTIMAL;
			attachmentInfo.resolveMode = ResolveModeBits::AVERAGE_BIT;
			attachmentInfo.resolveImageView = resolveResource.imageView;
			attachmentInfo.resolveImageLayout = Image::Layout::COLOUR_ATTACHMENT_OPTIMAL;
			attachmentInfo.loadOp = loadOp;
			attachmentInfo.storeOp = storeOp;
			attachmentInfo.clearValue = clearValue;
		}
		else if (resource.newState == Resource::State::DEPTH_STENCIL_ATTACHMENT)
		{
			RenderingAttachmentInfo& attachmentInfo = m_DepthAttachmentInfo;
			attachmentInfo.imageView = resource.imageView;
			attachmentInfo.imageLayout = Image::Layout::DEPTH_ATTACHMENT_OPTIMAL;
			attachmentInfo.resolveMode = ResolveModeBits::AVERAGE_BIT;
			attachmentInfo.resolveImageView = resolveResource.imageView;
			attachmentInfo.resolveImageLayout = Image::Layout::DEPTH_ATTACHMENT_OPTIMAL;
			attachmentInfo.loadOp = loadOp;
			attachmentInfo.storeOp = storeOp;
			attachmentInfo.clearValue = clearValue;

			m_RenderingInfo.pDepthAttachment = &m_DepthAttachmentInfo;
		}
		else
		{
			GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Resource::State is not COLOUR_ATTACHMENT or DEPTH_STENCIL_ATTACHMENT.");
		}

		m_OutputResources.push_back(resource);
		m_OutputResources.push_back(resolveResource);
	}
}

const PipelineRef& TaskPassParameters::GetPipeline() const 
{
	return m_RenderPipeline->GetPipeline();
}


void TaskPassParameters::SetRenderArea(Rect2D renderArea, uint32_t layers, uint32_t viewMask)
{
	m_RenderingInfo.renderArea = renderArea;
	m_RenderingInfo.layerCount = layers;
	m_RenderingInfo.viewMask = viewMask;
}

//////////////////////////
//TransferPassParameters//
//////////////////////////

TransferPassParameters::TransferPassParameters()
{
	m_Type = PassParameters::Type::TRANSFER;
}

TransferPassParameters::~TransferPassParameters()
{
	m_ResourcePairs.clear();
}

void TransferPassParameters::AddResource(const Ref<Vertexbuffer>& vertexbuffer)
{
	ResourceCopyRegion rcr;
	rcr.bufferCopy = { 0, 0, vertexbuffer->GetGPUBufferView()->GetCreateInfo().size };
	AddResourcePair(Resource(vertexbuffer->GetCPUBufferView(), Resource::State::TRANSFER_SRC), Resource(vertexbuffer->GetGPUBufferView(), Resource::State::TRANSFER_DST), rcr);
}

void TransferPassParameters::AddResource(const Ref<Indexbuffer>& indexbuffer)
{
	ResourceCopyRegion rcr;
	rcr.bufferCopy = { 0, 0, indexbuffer->GetGPUBufferView()->GetCreateInfo().size };
	AddResourcePair(Resource(indexbuffer->GetCPUBufferView(), Resource::State::TRANSFER_SRC), Resource(indexbuffer->GetGPUBufferView(), Resource::State::TRANSFER_DST), rcr);
}

void TransferPassParameters::AddResource(const Ref<BaseUniformbuffer>& uniformbuffer)
{
	ResourceCopyRegion rcr;
	rcr.bufferCopy = { 0, 0, uniformbuffer->GetGPUBufferView()->GetCreateInfo().size };
	AddResourcePair(Resource(uniformbuffer->GetCPUBufferView(), Resource::State::TRANSFER_SRC), Resource(uniformbuffer->GetGPUBufferView(), Resource::State::TRANSFER_DST), rcr);
}

void TransferPassParameters::AddResource(const Ref<BaseStoragebuffer>& storagebuffer)
{
	ResourceCopyRegion rcr;
	rcr.bufferCopy = { 0, 0, storagebuffer->GetGPUBufferView()->GetCreateInfo().size };
	AddResourcePair(Resource(storagebuffer->GetCPUBufferView(), Resource::State::TRANSFER_SRC), Resource(storagebuffer->GetGPUBufferView(), Resource::State::TRANSFER_DST), rcr);
}

void TransferPassParameters::AddResource(const Ref<Texture>& texture)
{
	ResourceCopyRegion rcr;
	rcr.bufferImageCopy =
	{
		0, 0, 0,
		{ texture->IsDepthTexture() ? Image::AspectBit::DEPTH_BIT : Image::AspectBit::COLOUR_BIT, 0, 0, texture->GetCreateInfo().arrayLayers },
		{ 0, 0, 0 },
		{ texture->GetWidth(), texture->GetHeight(), texture->GetDepth() }
	};
	AddResourcePair(Resource(texture->GetCPUBufferView(), Resource::State::TRANSFER_SRC), Resource(texture->GetImageView(), Resource::State::TRANSFER_DST), rcr);
}

void TransferPassParameters::AddResourcePair(const Resource& srcResource, const Resource& dstResource, const ResourceCopyRegion copyRegion)
{
	m_InputResources.push_back(srcResource);
	m_InputResources.back().newState = Resource::State::TRANSFER_SRC;
	m_OutputResources.push_back(dstResource);
	m_InputResources.back().newState = Resource::State::TRANSFER_DST;
	m_ResourcePairs.push_back({ srcResource, dstResource, copyRegion });
}