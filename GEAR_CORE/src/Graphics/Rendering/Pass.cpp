#include "gear_core_common.h"
#include "Graphics/Rendering/Pass.h"
#include "Graphics/Rendering/RenderGraph.h"

using namespace gear;
using namespace graphics;
using namespace rendering;

using namespace miru;
using namespace base;

////////
//Pass//
////////

Pass::Pass(const std::string& passName, const Ref<PassParameters>& passParameters, miru::base::CommandPool::QueueType queueType, RenderGraphPassFunction renderFunction)
	:m_PassName(passName), m_PassParameters(passParameters), m_QueueType(queueType), m_RenderFunction(renderFunction)
{
}

Pass::~Pass()
{
	m_RenderFunction = nullptr;
}

void Pass::Execute(RenderGraph* renderGraph, CommandBufferRef cmdBuffer, uint32_t frameIndex)
{
	
	if (GetPassParameters()->GetType() == PassParameters::Type::TASK)
	{
		ExecuteTask(renderGraph, cmdBuffer, frameIndex);
	}
	else if (GetPassParameters()->GetType() == PassParameters::Type::TRANSFER)
	{
		ExecuteTransfer(renderGraph, cmdBuffer, frameIndex);
	}
	else
	{
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Invalid PassParameter Type.");
	}
}

void Pass::ExecuteTask(RenderGraph* renderGraph, miru::base::CommandBufferRef cmdBuffer, uint32_t frameIndex)
{
	const CommandPool::QueueType cmdPoolType = cmdBuffer->GetCreateInfo().commandPool->GetCreateInfo().queueType;
	const Ref<TaskPassParameters>& passParameters = ref_cast<TaskPassParameters>(GetPassParameters());
	std::vector<ResourceView>& inputResourceViews = GetInputResourceViews();
	std::vector<ResourceView>& outputResourceViews = GetOutputResourceViews();
	const RenderingInfo& renderingInfo = passParameters->GetRenderingInfo();

	if (cmdPoolType != CommandPool::QueueType::TRANSFER)
	{
		cmdBuffer->BeginDebugLabel(frameIndex, m_PassName);
	}

	//Transition resources for the pass
	{
		CommandBuffer::DependencyInfo dependencyInfo = { DependencyBit::NONE_BIT, {} };
		std::vector<Barrier2Ref>& barriers = dependencyInfo.barriers;

		//Transition input and output resources
		barriers.clear();
		for (ResourceView& resourceView : inputResourceViews)
		{
			const auto& inputBarriers = TransitionResource(renderGraph, resourceView);
			barriers.insert(barriers.end(), inputBarriers.begin(), inputBarriers.end());
		}
		for (ResourceView& resourceView : outputResourceViews)
		{
			const auto& outputBarriers = TransitionResource(renderGraph, resourceView);
			barriers.insert(barriers.end(), outputBarriers.begin(), outputBarriers.end());
		}
		if (!barriers.empty())
		{
			cmdBuffer->PipelineBarrier2(frameIndex, dependencyInfo);
		}
	}

	if (m_BeginRendering)
	{
		const uint32_t& width = renderingInfo.renderArea.extent.width;
		const uint32_t& height = renderingInfo.renderArea.extent.height;

		cmdBuffer->BeginRendering(frameIndex, renderingInfo);
		cmdBuffer->SetViewport(frameIndex, { TaskPassParameters::CreateViewport(width, height) });
		cmdBuffer->SetScissor(frameIndex, { TaskPassParameters::CreateScissor(width, height) });
	}

	cmdBuffer->BindPipeline(frameIndex, passParameters->GetPipeline());
	cmdBuffer->BindDescriptorSets(frameIndex, passParameters->GetDescriptorSets(), 0, passParameters->GetPipeline());
	m_RenderFunction(cmdBuffer, frameIndex);

	if (m_EndRendering)
	{
		cmdBuffer->EndRendering(frameIndex);

		for (ResourceView& resourceView : outputResourceViews)
		{
			if (resourceView.IsImageView() && resourceView.imageView->IsSwapchainImageView())
			{
				CommandBuffer::DependencyInfo dependencyInfo = { DependencyBit::NONE_BIT, { TransitionResource(renderGraph, resourceView, Resource::State::PRESENT) } };
				cmdBuffer->PipelineBarrier2(frameIndex, dependencyInfo);
			}
		}
	}

	if (cmdPoolType != CommandPool::QueueType::TRANSFER)
	{
		cmdBuffer->EndDebugLabel(frameIndex);
	}
}

void Pass::ExecuteTransfer(RenderGraph* renderGraph, miru::base::CommandBufferRef cmdBuffer, uint32_t frameIndex)
{
	const CommandPool::QueueType cmdPoolType = cmdBuffer->GetCreateInfo().commandPool->GetCreateInfo().queueType;
	const Ref<TransferPassParameters>& passParameters = ref_cast<TransferPassParameters>(GetPassParameters());

	if (cmdPoolType != CommandPool::QueueType::TRANSFER)
	{
		cmdBuffer->BeginDebugLabel(frameIndex, m_PassName);
	}

	//Transition resource pairs
	CommandBuffer::DependencyInfo dependencyInfo = { DependencyBit::NONE_BIT, {} };
	std::vector<Barrier2Ref>& barriers = dependencyInfo.barriers;
	for (auto& [src, dst, rcr] : passParameters->GetResourceViewsPairs())
	{
		const auto& srcBarriers = TransitionResource(renderGraph, src);
		barriers.insert(barriers.end(), srcBarriers.begin(), srcBarriers.end());
		const auto& dstBarriers = TransitionResource(renderGraph, dst);
		barriers.insert(barriers.end(), dstBarriers.begin(), dstBarriers.end());
	}
	cmdBuffer->PipelineBarrier2(frameIndex, dependencyInfo);

	//Transfer resources
	for (auto& [src, dst, rcr] : passParameters->GetResourceViewsPairs())
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

Pass::TransitionDetails Pass::GetTransitionDetails(const Resource::State& state, const PipelineStageBit& stage, bool src)
{
	Barrier::AccessBit accesses = Barrier::AccessBit::NONE_BIT;
	Image::Layout layout = Image::Layout::UNKNOWN;
	PipelineStageBit pipelineStage = PipelineStageBit::NONE_BIT;

	switch (state)
	{
	default:
	case Resource::State::UNKNOWN:
	{
		return { accesses, layout, pipelineStage };
	}
	case Resource::State::PRESENT:
	{
		accesses = src ? Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT : Barrier::AccessBit::NONE_BIT;
		layout = Image::Layout::PRESENT_SRC;
		pipelineStage = GraphicsAPI::IsD3D12() ? PipelineStageBit::NONE_BIT : PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT;
		break;
	}
	case Resource::State::COLOUR_ATTACHMENT:
	{
		accesses = src ? Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT : Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT;
		layout = Image::Layout::COLOUR_ATTACHMENT_OPTIMAL;
		pipelineStage = PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT;
		break;
	}
	case Resource::State::DEPTH_STENCIL_ATTACHMENT:
	{
		accesses = (src || GraphicsAPI::IsD3D12()) ? Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		layout = Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		pipelineStage = PipelineStageBit::EARLY_FRAGMENT_TESTS_BIT | PipelineStageBit::LATE_FRAGMENT_TESTS_BIT;
		break;
	}
	case Resource::State::VERTEX_BUFFER:
	{
		accesses = src ? Barrier::AccessBit::NONE_BIT : Barrier::AccessBit::VERTEX_ATTRIBUTE_READ_BIT;
		pipelineStage = src ? stage : GraphicsAPI::IsD3D12() ? PipelineStageBit::VERTEX_SHADER_BIT : PipelineStageBit::VERTEX_INPUT_BIT;
		break;
	}
	case Resource::State::INDEX_BUFFER:
	{
		accesses = src ? Barrier::AccessBit::NONE_BIT : Barrier::AccessBit::INDEX_READ_BIT;
		pipelineStage = src ? stage : PipelineStageBit::VERTEX_INPUT_BIT;
		break;
	}
	case Resource::State::UNIFORM_BUFFER:
	{
		accesses = src ? Barrier::AccessBit::NONE_BIT : Barrier::AccessBit::UNIFORM_READ_BIT;
		pipelineStage = GraphicsAPI::IsD3D12() && src ? PipelineStageBit::ALL_COMMANDS_BIT : stage;
		break;
	}
	case Resource::State::SHADER_READ_ONLY:
	{
		accesses = GraphicsAPI::IsD3D12() ? Barrier::AccessBit::SHADER_READ_BIT : (src ? Barrier::AccessBit::NONE_BIT : Barrier::AccessBit::SHADER_READ_BIT);
		layout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		pipelineStage = GraphicsAPI::IsD3D12() && src ? PipelineStageBit::ALL_COMMANDS_BIT : stage;
		break;
	}
	case Resource::State::SHADER_READ_WRITE:
	{
		accesses = GraphicsAPI::IsD3D12() ? Barrier::AccessBit::SHADER_WRITE_BIT : (src ? Barrier::AccessBit::NONE_BIT : (Barrier::AccessBit::SHADER_WRITE_BIT | Barrier::AccessBit::SHADER_READ_BIT));
		layout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
		pipelineStage = GraphicsAPI::IsD3D12() && src ? PipelineStageBit::ALL_COMMANDS_BIT : stage;
		break;
	}
	case Resource::State::TRANSFER_SRC:
	{
		accesses = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		layout = Image::Layout::TRANSFER_SRC_OPTIMAL;
		pipelineStage = PipelineStageBit::TRANSFER_BIT;
		break;
	}
	case Resource::State::TRANSFER_DST:
	{
		accesses = Barrier::AccessBit::TRANSFER_READ_BIT;
		layout = Image::Layout::TRANSFER_DST_OPTIMAL;
		pipelineStage = PipelineStageBit::TRANSFER_BIT;
		break;
	}
	}
	return { accesses, layout, pipelineStage };
}

std::vector<Barrier2Ref> Pass::TransitionResource(RenderGraph* renderGraph, const ResourceView& passResourceView, Resource::State overideNewState)
{
	Barrier::Type type;
	if (passResourceView.IsImageView())
	{
		type = Barrier::Type::IMAGE;
	}
	else if (passResourceView.IsBufferView())
	{
		type = Barrier::Type::BUFFER;
	}
	else
	{
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_STATE, "ResourceView is neither an ImageView or a BufferView.");
	}

	Resource& resource = renderGraph->GetTrackedResource(passResourceView.GetResource());

	Resource::State newState = passResourceView.state;
	const PipelineStageBit& stage = passResourceView.pipelineStage;
	const DescriptorType& descriptorType = passResourceView.descriptorType;
	if (overideNewState != Resource::State::UNKNOWN)
		newState = overideNewState;

	const Image::SubresourceRange& subresourceRange = type == Barrier::Type::IMAGE ? passResourceView.imageView->GetCreateInfo().subresourceRange : Image::SubresourceRange();

	std::vector<Image::SubresourceRange> subresourceRanges;
	if (type == Barrier::Type::IMAGE)
	{
		if (resource.AreSubresourcesInSameState(subresourceRange))
		{
			subresourceRanges.push_back(subresourceRange);
		}
		else
		{
			for (auto& subresource : resource.GetSubresourcesToTransition(newState, subresourceRange))
			{
				subresourceRanges.emplace_back(subresourceRange.aspect, subresource.first, 1, subresource.second, 1);
			}
		}
	}
	else
	{
		subresourceRanges.push_back(subresourceRange);
	}

	std::vector<Barrier2Ref> barriers;
	for (const auto& _subresourceRange : subresourceRanges)
	{	
		const Resource::State& oldState = type == Barrier::Type::IMAGE ? resource.GetSubresources(_subresourceRange) : resource.GetSubresources();
		if (oldState == newState)
			continue;
		
		const TransitionDetails& src = GetTransitionDetails(oldState, stage, true);
		const TransitionDetails& dst = GetTransitionDetails(newState, stage, false);

		Barrier2::CreateInfo barrierCI;
		barrierCI.type = type;
		barrierCI.srcStageMask = src.pipelineStage;
		barrierCI.srcAccess = src.accesses;
		barrierCI.dstStageMask = dst.pipelineStage;
		barrierCI.dstAccess = dst.accesses;
		barrierCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		barrierCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		if (type == Barrier::Type::BUFFER)
		{
			barrierCI.buffer = passResourceView.bufferView->GetCreateInfo().buffer;
			barrierCI.offset = passResourceView.bufferView->GetCreateInfo().offset;
			barrierCI.size = passResourceView.bufferView->GetCreateInfo().size;
		}
		if (type == Barrier::Type::IMAGE)
		{
			barrierCI.image = passResourceView.imageView->GetCreateInfo().image;
			barrierCI.oldLayout = src.layout;
			barrierCI.newLayout = dst.layout;
			barrierCI.subresourceRange = _subresourceRange;
		}
		barriers.push_back(Barrier2::Create(&barrierCI));

#if 0
		constexpr auto resourceStates = magic_enum::enum_names<Resource::State>();
		std::string logMessage = "\n";
		logMessage += "Resource: " + std::string((type == Barrier::Type::IMAGE) ? passResourceView.imageView->GetCreateInfo().debugName : passResourceView.bufferView->GetCreateInfo().debugName) + "\n";
		logMessage += "Type: " + std::string((type == Barrier::Type::IMAGE) ? "IMAGE" : "BUFFER") + "\n";
		logMessage += "\t" + std::string(resourceStates[static_cast<size_t>(oldState)]) + "\n";
		logMessage += "\tsrc.pipelineStage: " + std::to_string((uint64_t)src.pipelineStage) + "\n";
		logMessage += "\tsrc.accesses: " + std::to_string((uint64_t)src.accesses) + "\n";
		logMessage += (type == Barrier::Type::IMAGE) ? "\tsrc.layout: " + std::to_string((uint64_t)src.layout) + "\n" : "";
		logMessage += "\t" + std::string(resourceStates[static_cast<size_t>(newState)]) + "\n";
		logMessage += "\tdst.pipelineStage: " + std::to_string((uint64_t)dst.pipelineStage) + "\n";
		logMessage += "\tdst.accesses: " + std::to_string((uint64_t)dst.accesses) + "\n";
		logMessage += (type == Barrier::Type::IMAGE) ? "\tdst.layout: " + std::to_string((uint64_t)dst.layout) + "\n" : "";

		GEAR_INFO(ErrorCode::GRAPHICS, logMessage.c_str());
#endif

		type == Barrier::Type::IMAGE ? resource.SetSubresources(newState, _subresourceRange) : resource.SetSubresources(newState);
	}

	return barriers;
}