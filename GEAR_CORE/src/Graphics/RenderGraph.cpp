#include "gear_core_common.h"

#include "Graphics/RenderGraph.h"
#include "Graphics/AllocatorManager.h"

#include "Objects/Camera.h"
#include "Objects/Skybox.h"
#include "Objects/Light.h"
#include "Objects/Model.h"
#include "Objects/Material.h"

using namespace gear;
using namespace graphics;
using namespace graphics;

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
	type = DescriptorType(-1);
	imageView = texture->GetImageView();

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

bool Resource::operator== (const Resource& other) const
{
	if (type == other.type)
	{
		switch (type)
		{
		case DescriptorType::SAMPLER:
			return sampler == other.sampler;
		case DescriptorType::COMBINED_IMAGE_SAMPLER:
			return (sampler == other.sampler) && (imageView == other.imageView);
		case DescriptorType::SAMPLED_IMAGE:
		case DescriptorType::STORAGE_IMAGE:
			return imageView == other.imageView;
		case DescriptorType::UNIFORM_TEXEL_BUFFER:
		case DescriptorType::STORAGE_TEXEL_BUFFER:
		case DescriptorType::UNIFORM_BUFFER:
		case DescriptorType::STORAGE_BUFFER:
		case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
		case DescriptorType::STORAGE_BUFFER_DYNAMIC:
			return bufferView == other.bufferView;
		case DescriptorType::INPUT_ATTACHMENT:
			return imageView == other.imageView;
		case DescriptorType::ACCELERATION_STRUCTURE:
			return accelerationStructure == other.accelerationStructure;
		default:
		{
			if (IsImageView())
				return imageView == other.imageView;
			else if (IsSampler())
				return sampler == other.sampler;
			else if (IsBufferView())
				return bufferView == other.bufferView;
			else if (IsAccelerationStructure())
				return accelerationStructure == other.accelerationStructure;
			else
				return false;

		}
		}
	}

	return false;
}

bool Resource::operator!= (const Resource& other) const
{
	return !(*this == other);
}

//////////////////
//PassParameters//
//////////////////

PassParameters::~PassParameters()
{
	m_InputResources.clear();
	m_OutputResources.clear();
}

////////////////////////
//RenderPassParameters//
////////////////////////

RenderPassParameters::RenderPassParameters(const Ref<RenderPipeline>& renderPipeline, const std::vector<size_t>& setMultiplers)
{
	m_Type = PassParameters::Type::RENDER;
	m_RenderPipeline = renderPipeline;

	const RenderPipeline::ResourceBindingDescriptions& rbds = m_RenderPipeline->GetRBDs();
	std::map<DescriptorType, uint32_t> poolSizesMap;
	uint32_t maxSets = 0;

	for (uint32_t set = 0; set < static_cast<uint32_t>(rbds.size()); set++)
	{
		for (uint32_t binding = 0; binding < static_cast<uint32_t>(rbds[set].size()); binding++)
		{
			const Shader::ResourceBindingDescription& rbd = rbds[set][binding];

			uint32_t setMultipler = 1;
			if (!setMultiplers.empty() && set < setMultiplers.size())
				setMultipler = static_cast<uint32_t>(setMultiplers[set]);

			poolSizesMap[rbd.type] += (rbd.descriptorCount * setMultipler);
			maxSets += setMultipler;
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
		uint32_t setMultipler = 1;
		if (!setMultiplers.empty() && set < setMultiplers.size())
			setMultipler = static_cast<uint32_t>(setMultiplers[set]);

		for (uint32_t setIndex = 0; setIndex < setMultipler; setIndex++)
		{
			descSetPerViewCI.debugName = "GEAR_CORE_DescriptorSet_RenderGraph_" + m_RenderPipeline->GetCreateInfo().debugName + " Set: " + std::to_string(set) + " Set Index: " + std::to_string(setIndex);
			descSetPerViewCI.descriptorSetLayouts = { m_RenderPipeline->GetDescriptorSetLayouts()[set] };
			m_DescriptorSets[set][setIndex] = DescriptorSet::Create(&descSetPerViewCI);
		}
	}

	m_RenderingInfo.flags = RenderingFlagBits::NONE_BIT;
	m_RenderingInfo.colourAttachments.resize(m_RenderPipeline->GetCreateInfo().dynamicRendering.colourAttachmentFormats.size());
	m_RenderingInfo.pDepthAttachment = nullptr;
	m_RenderingInfo.pStencilAttachment = nullptr;
}

RenderPassParameters::~RenderPassParameters()
{
	for (auto& descSet : m_DescriptorSets)
		descSet.second.clear();

	m_DescriptorSets.clear();
}

void RenderPassParameters::Setup()
{
	for (auto& descSet : m_DescriptorSets)
		for (auto& descSetIndex : descSet.second)
			descSetIndex.second->Update();
}

const std::pair<uint32_t, uint32_t> RenderPassParameters::FindResourceSetBinding(const std::string& name) const
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

void RenderPassParameters::SetResource(const std::pair<uint32_t, uint32_t>& set_binding, const Resource& resource, uint32_t setIndex)
{
	const uint32_t& set = set_binding.first;
	const uint32_t& binding = set_binding.second;

	const Shader::ResourceBindingDescription& rbd = m_RenderPipeline->GetRBDs()[set][binding];
	if (rbd.type != resource.type)
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "The Resource DescriptorType does not match ResourceBindingDescription DescriptorType.");
	}

	Resource _resource = resource;

	switch (resource.type)
	{
	case DescriptorType::SAMPLER:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = resource.sampler;
		info.imageView = nullptr;
		info.imageLayout = Image::Layout(0);
		m_DescriptorSets[set][setIndex]->AddImage(setIndex, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::COMBINED_IMAGE_SAMPLER:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = resource.sampler;
		info.imageView = resource.imageView;
		info.imageLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorSets[set][setIndex]->AddImage(setIndex, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::SAMPLED_IMAGE:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = nullptr;
		info.imageView = resource.imageView;
		info.imageLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorSets[set][setIndex]->AddImage(setIndex, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::STORAGE_IMAGE:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = nullptr;
		info.imageView = resource.imageView;
		info.imageLayout = Image::Layout::GENERAL;
		m_DescriptorSets[set][setIndex]->AddImage(setIndex, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_WRITE;
		break;
	}
	case DescriptorType::UNIFORM_TEXEL_BUFFER:
	case DescriptorType::UNIFORM_BUFFER:
	case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
	{
		DescriptorSet::DescriptorBufferInfo info;
		info.bufferView = resource.bufferView;
		m_DescriptorSets[set][setIndex]->AddBuffer(setIndex, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::STORAGE_TEXEL_BUFFER:
	case DescriptorType::STORAGE_BUFFER:
	case DescriptorType::STORAGE_BUFFER_DYNAMIC:
	{
		DescriptorSet::DescriptorBufferInfo info;
		info.bufferView = resource.bufferView;
		m_DescriptorSets[set][setIndex]->AddBuffer(setIndex, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_WRITE;
		break;
	}
	case DescriptorType::INPUT_ATTACHMENT:
	{
		DescriptorSet::DescriptorImageInfo info;
		info.sampler = nullptr;
		info.imageView = resource.imageView;
		info.imageLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorSets[set][setIndex]->AddImage(setIndex, binding, { info });

		_resource.newState = Resource::State::SHADER_READ_ONLY;
		break;
	}
	case DescriptorType::ACCELERATION_STRUCTURE:
	{
		m_DescriptorSets[set][setIndex]->AddAccelerationStructure(setIndex, binding, { resource.accelerationStructure });

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

void RenderPassParameters::SetResource(const std::string& name, const Resource& resource, uint32_t setIndex)
{
	SetResource(FindResourceSetBinding(name), resource, setIndex);
}

void RenderPassParameters::AddAttachment(uint32_t index, const Resource& resource, RenderPass::AttachmentLoadOp loadOp, RenderPass::AttachmentStoreOp storeOp, const Image::ClearValue& clearValue)
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

void RenderPassParameters::AddAttachmentWithResolve(uint32_t index, const Resource& resource, const Resource& resolveResource, RenderPass::AttachmentLoadOp loadOp, RenderPass::AttachmentStoreOp storeOp, const Image::ClearValue& clearValue)
{
	if (resource.imageView && resolveResource.imageView)
	{
		if (resource.newState == Resource::State::COLOUR_ATTACHMENT)
		{
			RenderingAttachmentInfo& attachmentInfo = m_RenderingInfo.colourAttachments[index];
			attachmentInfo.imageView = resource.imageView;
			attachmentInfo.imageLayout = Image::Layout::COLOUR_ATTACHMENT_OPTIMAL;
			attachmentInfo.resolveMode = ResolveModeBits::NONE_BIT;
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
			attachmentInfo.resolveMode = ResolveModeBits::NONE_BIT;
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
	}
}

void RenderPassParameters::SetRenderArea(Rect2D renderArea, uint32_t layers, uint32_t viewMask)
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

////////
//Pass//
////////

Pass::Pass(const std::string& passName, const Ref<PassParameters>& passParameters, miru::base::CommandPool::QueueType queueType, RenderGraphPassFunction renderFunction)
	:m_PassName(passName), m_PassParameters(passParameters), m_QueueType(queueType), m_RenderFunction(renderFunction)
{
}

Pass::~Pass()
{
	m_BackwardGraphicsDependentPasses.clear();
	m_ForwardGraphicsDependentPasses.clear();
	m_RenderFunction = nullptr;
}

void Pass::Execute(CommandBufferRef cmdBuffer, uint32_t frameIndex)
{
	const CommandPool::QueueType cmdPoolType = cmdBuffer->GetCreateInfo().commandPool->GetCreateInfo().queueType;
	if (GetPassParameters()->GetType() == PassParameters::Type::RENDER)
	{
		const Ref<RenderPassParameters>& passParameters = ref_cast<RenderPassParameters>(GetPassParameters());
		const RenderingInfo& renderingInfo = passParameters->GetRenderingInfo();

		bool graphics = renderingInfo.colourAttachments.size();
		bool noBackwardDependencise = m_BackwardGraphicsDependentPasses.empty();
		bool noForwardDependencies = m_ForwardGraphicsDependentPasses.empty();

		if (cmdPoolType != CommandPool::QueueType::TRANSFER)
		{
			cmdBuffer->BeginDebugLabel(frameIndex, m_PassName);
		}

		if (graphics && noBackwardDependencise)
		{
			std::vector<BarrierRef> barriers;
			for (Resource& resource : GetOutputResources())
				barriers.push_back(TransitionResource(resource));
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, DependencyBit::NONE_BIT, barriers);

			const uint32_t& width = renderingInfo.renderArea.extent.width;
			const uint32_t& height = renderingInfo.renderArea.extent.height;

			cmdBuffer->BeginRendering(frameIndex, renderingInfo);
			cmdBuffer->SetViewport(frameIndex, { RenderPassParameters::CreateViewport(width, height) });
			cmdBuffer->SetScissor(frameIndex, { RenderPassParameters::CreateScissor(width, height) });
		}
		cmdBuffer->BindPipeline(frameIndex, passParameters->GetPipeline());
		m_RenderFunction(cmdBuffer, frameIndex);
		if (graphics && noForwardDependencies)
		{
			cmdBuffer->EndRendering(frameIndex);

			for (Resource& resource : GetOutputResources())
			{
				if (resource.IsImageView() && resource.imageView->IsSwapchainImageView())
				{
					resource.newState = Resource::State::PRESENT;
					cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::BOTTOM_OF_PIPE_BIT, DependencyBit::NONE_BIT, { TransitionResource(resource) });
				}
			}
		}

		if (cmdPoolType != CommandPool::QueueType::TRANSFER)
		{
			cmdBuffer->EndDebugLabel(frameIndex);
		}
	}
	else if (GetPassParameters()->GetType() == PassParameters::Type::TRANSFER)
	{
		const Ref<TransferPassParameters>& passParameters = ref_cast<TransferPassParameters>(GetPassParameters());

		if (cmdPoolType != CommandPool::QueueType::TRANSFER)
		{
			cmdBuffer->BeginDebugLabel(frameIndex, m_PassName);
		}

		for (const auto& [src, dst, rcr] : passParameters->GetResourcesPairs())
		{
			const bool& buffer_buffer = src.IsBufferView() && dst.IsBufferView();
			const bool& buffer_image = src.IsBufferView() && dst.IsImageView();
			const bool& image_buffer = src.IsImageView() && dst.IsBufferView();
			const bool& image_image = src.IsImageView() && dst.IsImageView();

			if (buffer_buffer)
			{
				const BufferRef& srcBuffer = src.bufferView->GetCreateInfo().buffer;
				const BufferRef& dstBuffer = dst.bufferView->GetCreateInfo().buffer;
				const Buffer::Copy& copyRegion = rcr.bufferCopy;
				cmdBuffer->CopyBuffer(frameIndex, srcBuffer, dstBuffer, { copyRegion });
			}
			if (buffer_image)
			{
				const BufferRef& srcBuffer = src.bufferView->GetCreateInfo().buffer;
				const ImageRef& dstImage = dst.imageView->GetCreateInfo().image;
				const Image::BufferImageCopy& copyRegion = rcr.bufferImageCopy;
				cmdBuffer->CopyBufferToImage(frameIndex, srcBuffer, dstImage, Image::Layout::TRANSFER_DST_OPTIMAL, { copyRegion });
			}
			if (image_buffer)
			{
				const ImageRef& srcImage = src.imageView->GetCreateInfo().image;
				const BufferRef& dstBuffer = dst.bufferView->GetCreateInfo().buffer;
				const Image::BufferImageCopy& copyRegion = rcr.bufferImageCopy;
				cmdBuffer->CopyImageToBuffer(frameIndex, srcImage, dstBuffer, Image::Layout::TRANSFER_SRC_OPTIMAL, { copyRegion });
			}
			if (image_image)
			{
				const ImageRef& srcImage = src.imageView->GetCreateInfo().image;
				const ImageRef& dstImage = dst.imageView->GetCreateInfo().image;
				const Image::Copy& copyRegion = rcr.imageCopy;
				cmdBuffer->CopyImage(frameIndex, srcImage, Image::Layout::TRANSFER_SRC_OPTIMAL, dstImage, Image::Layout::TRANSFER_DST_OPTIMAL, { copyRegion });
			}
		}

		if (cmdPoolType != CommandPool::QueueType::TRANSFER)
		{
			cmdBuffer->EndDebugLabel(frameIndex);
		}
	}
	else
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Invalid PassParameter Type.");
	}
}

BarrierRef Pass::TransitionResource(Resource& resource)
{
	const bool& srcComputeQueue = false;
	const bool& dstComputeQueue = false;

	Resource::State& oldState = resource.oldState;
	Resource::State& newState = resource.newState;

	Barrier::Type type;
	if (resource.IsImageView())
		type = Barrier::Type::IMAGE;
	else if (resource.IsBufferView())
		type = Barrier::Type::BUFFER;
	else
		return nullptr;

	auto GetAccessesAndLayouts = [](Resource::State state, bool src, bool computeQueue) -> std::pair<Barrier::AccessBit, Image::Layout>
	{
		Barrier::AccessBit accesses = Barrier::AccessBit::NONE_BIT;
		Image::Layout layout = Image::Layout::UNKNOWN;

		switch (state)
		{
		default:
		case Resource::State::UNKNOWN:
			break;
		case Resource::State::PRESENT:
		{
			if (src)
				accesses = Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT;
			else
				accesses = Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT;
			layout = Image::Layout::PRESENT_SRC;
			break;
		}
		case Resource::State::COLOUR_ATTACHMENT:
		{
			if (src)
				accesses = Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT;
			else
				accesses = Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT;
			layout = Image::Layout::COLOUR_ATTACHMENT_OPTIMAL;
			break;
		}
		case Resource::State::DEPTH_STENCIL_ATTACHMENT:
		{
			if (src)
				accesses = Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			else
				accesses = Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			layout = Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;
		}
		case Resource::State::SHADER_READ_ONLY:
		{
			if (src)
				accesses = Barrier::AccessBit::SHADER_WRITE_BIT;
			else
				accesses = Barrier::AccessBit::SHADER_READ_BIT;

			if (computeQueue && GraphicsAPI::IsD3D12())
				layout = Image::Layout::D3D12_NON_PIXEL_SHADER_READ_ONLY_OPTIMAL;
			else
				layout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
			break;
		}
		case Resource::State::SHADER_READ_WRITE:
		{
			accesses = Barrier::AccessBit::SHADER_WRITE_BIT | Barrier::AccessBit::SHADER_READ_BIT;
			layout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
			break;
		}
		case Resource::State::TRANSFER_SRC:
		{
			accesses = Barrier::AccessBit::TRANSFER_WRITE_BIT;
			layout = Image::Layout::TRANSFER_SRC_OPTIMAL;
			break;
		}
		case Resource::State::TRANSFER_DST:
		{
			accesses = Barrier::AccessBit::TRANSFER_READ_BIT;
			layout = Image::Layout::TRANSFER_SRC_OPTIMAL;
			break;
		}
		}

		return { accesses, layout };
	};
	
	auto src = GetAccessesAndLayouts(oldState, true, srcComputeQueue);
	auto dst = GetAccessesAndLayouts(newState, false, dstComputeQueue);

	oldState = newState;

	Barrier::CreateInfo barrierCI;
	barrierCI.type = type;
	barrierCI.srcAccess = src.first;
	barrierCI.dstAccess = dst.first;
	barrierCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
	barrierCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
	if (type == Barrier::Type::BUFFER)
	{
		barrierCI.buffer = resource.bufferView->GetCreateInfo().buffer;
		barrierCI.offset = resource.bufferView->GetCreateInfo().offset;
		barrierCI.size = resource.bufferView->GetCreateInfo().size;
	}
	if (type == Barrier::Type::IMAGE)
	{
		barrierCI.image = resource.imageView->GetCreateInfo().image;
		barrierCI.oldLayout = src.second;
		barrierCI.newLayout = dst.second;
		barrierCI.subresourceRange = resource.imageView->GetCreateInfo().subresourceRange;
	}
	return Barrier::Create(&barrierCI);
}

///////////////
//RenderGraph//
///////////////

RenderGraph::RenderGraph(const ContextRef& context, uint32_t commandBufferCount)
{
	m_Context = context;

	//CmdPools and CmdBuffers
	for (uint32_t i = 0; i < 3; i++)
	{
		CommandPool::QueueType type = CommandPool::QueueType(i);
		std::string name;
		switch (type)
		{
		case CommandPool::QueueType::GRAPHICS:
			name = "Graphics";
			break;
		case CommandPool::QueueType::COMPUTE:
			name = "Compute";
			break;
		case CommandPool::QueueType::TRANSFER:
			name = "Transfer";
			break;
		default:
			break;
		}

		auto& cmd = m_CommandPoolAndBuffers[type];
		cmd.cmdPoolCI.debugName = "GEAR_CORE_CommandPool_RenderGraph_" + name;
		cmd.cmdPoolCI.context = m_Context;
		cmd.cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
		cmd.cmdPoolCI.queueType = type;
		cmd.cmdPool = CommandPool::Create(&cmd.cmdPoolCI);

		cmd.cmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_RenderGraph_" + name;
		cmd.cmdBufferCI.commandPool = cmd.cmdPool;
		cmd.cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
		cmd.cmdBufferCI.commandBufferCount = commandBufferCount;
		cmd.cmdBuffer = CommandBuffer::Create(&cmd.cmdBufferCI);
	}

}

RenderGraph::~RenderGraph()
{
	Reset();
}

Ref<Pass> RenderGraph::AddPass(const std::string& passName, const Ref<PassParameters>& passParameters, CommandPool::QueueType queueType, RenderGraphPassFunction renderFunction)
{
	//Create and Setup Pass
	Ref<Pass> pass = CreateRef<Pass>(passName, passParameters, queueType, renderFunction);
	pass->GetPassParameters()->Setup();
	
	//Search and Resolve any multiple write dependencies
	for (auto& previousPass : m_Passes)
	{
		if (previousPass->GetOutputResources() == pass->GetOutputResources())
		{
			pass->m_BackwardGraphicsDependentPasses.push_back(previousPass);
			previousPass->m_ForwardGraphicsDependentPasses.push_back(pass);
		}
	}

	//Add Pass Resources to the RenderGraph 'master' list
	for (Resource& resource : pass->GetInputResources())
	{
		if (arc::FindInVector(m_Resources, resource))
			m_Resources.push_back(resource);
	}
	for (Resource& resource : pass->GetOutputResources())
	{
		if (arc::FindInVector(m_Resources, resource))
			m_Resources.push_back(resource);
	}

	//Add pass to the RenderGraph
	m_Passes.push_back(pass);
	pass->m_UnorderedListIndex = m_Passes.size() - 1;
	return pass;
}

void RenderGraph::Compile()
{
	//Organizing GPU Work with Directed Acyclic Graphs
	//https://levelup.gitconnected.com/organizing-gpu-work-with-directed-acyclic-graphs-f3fd5f2c2af3

	//Build Adjacency Lists
	{
		m_AdjacencyLists.resize(m_Passes.size());
		for (size_t passIndex = 0; passIndex < m_Passes.size(); passIndex++)
		{
			Ref<Pass>& pass = m_Passes[passIndex];
			std::vector<uint64_t>& adjacentPassIndices = m_AdjacencyLists[passIndex];

			for (size_t otherPassIndex = 0; otherPassIndex < m_Passes.size(); otherPassIndex++)
			{
				if (passIndex == otherPassIndex)
					continue; // Do not check dependencies on itself

				Ref<Pass>& otherPass = m_Passes[otherPassIndex];

				for (auto& otherPassReadResource : otherPass->GetInputResources())
				{
					bool otherPassDependsOnCurrentPass = arc::FindInVector(pass->GetOutputResources(), otherPassReadResource);
					if (otherPassDependsOnCurrentPass)
					{
						adjacentPassIndices.push_back(otherPassIndex);
					}
				}
			}
		}
	}

	//Topological Sort
	{
		std::function<void(size_t, std::vector<bool>&, std::vector<bool>&)> DepthFirstSearch
			= [&](size_t passIndex, std::vector<bool>& visited, std::vector<bool>& onStack)
		{
			visited[passIndex] = true;
			onStack[passIndex] = true;

			size_t adjacencyListIndex = m_Passes[passIndex]->m_UnorderedListIndex;
			for (size_t neighbour : m_AdjacencyLists[adjacencyListIndex])
			{
				if (!visited[neighbour])
				{
					DepthFirstSearch(neighbour, visited, onStack);
				}
			}

			onStack[passIndex] = false;
			m_TopologicallySortedPasses.push_back(m_Passes[passIndex]);
		};

		std::vector<bool> visitedPasses(m_Passes.size(), false);
		std::vector<bool> onStackPasses(m_Passes.size(), false);

		for (size_t passIndex = 0; passIndex < m_Passes.size(); passIndex++)
		{
			if (!visitedPasses[passIndex])
			{
				DepthFirstSearch(passIndex, visitedPasses, onStackPasses);
			}
		}

		std::reverse(m_TopologicallySortedPasses.begin(), m_TopologicallySortedPasses.end());
	}

	//Build Dependency Levels
	{
		std::vector<size_t> longestDistances(m_TopologicallySortedPasses.size(), 0);
		size_t dependencyLevelCount = 1;

		for (size_t passIndex = 0; passIndex < m_Passes.size(); passIndex++)
		{
			size_t originalIndex = m_TopologicallySortedPasses[passIndex]->m_UnorderedListIndex;
			size_t adjacencyListIndex = originalIndex;

			for (size_t adjacentPassIndex : m_AdjacencyLists[adjacencyListIndex])
			{
				if (longestDistances[adjacentPassIndex] < longestDistances[originalIndex] + 1)
				{
					size_t newLongestDistance = longestDistances[originalIndex] + 1;
					longestDistances[adjacentPassIndex] = newLongestDistance;
					dependencyLevelCount = std::max(size_t(newLongestDistance + 1), dependencyLevelCount);
				}
			}
		}

		m_DependencyLevels.resize(dependencyLevelCount);
		for (size_t passIndex = 0; passIndex < m_Passes.size(); passIndex++)
		{
			Ref<Pass>& pass = m_Passes[passIndex];
			uint64_t levelIndex = longestDistances[passIndex];
			DependencyLevel& dependencyLevel = m_DependencyLevels[levelIndex];
			dependencyLevel.m_Passes.push_back(pass);
			dependencyLevel.m_LevelIndex = levelIndex;
			pass->m_DependencyLevelIndex = levelIndex;
		}
	}
}

void RenderGraph::Execute(uint32_t frameIndex)
{
	Compile();

	CommandBufferRef& cmdBuffer = GetCommandBuffer(CommandPool::QueueType::GRAPHICS/*pass->m_QueueType*/);
	cmdBuffer->Reset(frameIndex, false);
	cmdBuffer->Begin(frameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
	for (const auto& pass : m_TopologicallySortedPasses)
	{
		pass->Execute(cmdBuffer, frameIndex);
	}
	cmdBuffer->End(frameIndex);
}

void RenderGraph::Reset()
{
	m_DependencyLevels.clear();
	m_AdjacencyLists.clear();
	m_TopologicallySortedPasses.clear();
	m_Resources.clear();
	m_Passes.clear();
}

ImageRef RenderGraph::CreateImage(const ImageDesc& desc, const std::string& name)
{
	Image::UsageBit usage = Image::UsageBit(0);
	if (arc::BitwiseCheck(desc.usage, ImageDesc::UsageBit::COLOUR_ATTACHMENT))
		usage |= Image::UsageBit::COLOUR_ATTACHMENT_BIT;
	if (arc::BitwiseCheck(desc.usage, ImageDesc::UsageBit::DEPTH_STENCIL_ATTACHMENT))
		usage |= Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
	if (arc::BitwiseCheck(desc.usage, ImageDesc::UsageBit::SHADER_READ_ONLY))
		usage |= Image::UsageBit::SAMPLED_BIT;
	if (arc::BitwiseCheck(desc.usage, ImageDesc::UsageBit::SHADER_READ_WRITE))
		usage |= Image::UsageBit::STORAGE_BIT;

	Image::CreateInfo imageCI;
	imageCI.debugName = "GEAR_CORE_Image_RenderGraph_" + name;
	imageCI.device = m_Context->GetDevice();
	imageCI.type = desc.type;
	imageCI.format = desc.format;
	imageCI.width = desc.width;
	imageCI.height = desc.height;
	imageCI.depth = desc.depth;
	imageCI.mipLevels = desc.mipLevels;
	imageCI.arrayLayers = desc.arrayLayers;
	imageCI.sampleCount = desc.sampleCount;
	imageCI.usage = usage;
	imageCI.layout = Image::Layout::UNKNOWN;
	imageCI.size = 0;
	imageCI.data = nullptr;
	imageCI.allocator = AllocatorManager::GetGPUAllocator();

	return Image::Create(&imageCI);
}

ImageViewRef RenderGraph::CreateImageView(const ImageRef& image, Image::Type type, const Image::SubresourceRange& subresourceRange)
{
	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "GEAR_CORE_ImageView_RenderGraph: " + image->GetCreateInfo().debugName;
	imageViewCI.device = m_Context->GetDevice();
	imageViewCI.image = image;
	imageViewCI.viewType = type;
	imageViewCI.subresourceRange = subresourceRange;

	return ImageView::Create(&imageViewCI);
}

CommandBufferRef& RenderGraph::GetCommandBuffer(CommandPool::QueueType queueType)
{
	return m_CommandPoolAndBuffers[queueType].cmdBuffer;
}