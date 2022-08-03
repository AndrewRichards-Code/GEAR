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
	m_BackwardGraphicsDependentPasses.clear();
	m_ForwardGraphicsDependentPasses.clear();
	m_RenderFunction = nullptr;
}

void Pass::Execute(RenderGraph* renderGraph, CommandBufferRef cmdBuffer, uint32_t frameIndex)
{
	const CommandPool::QueueType cmdPoolType = cmdBuffer->GetCreateInfo().commandPool->GetCreateInfo().queueType;
	if (GetPassParameters()->GetType() == PassParameters::Type::TASK)
	{
		const Ref<TaskPassParameters>& passParameters = ref_cast<TaskPassParameters>(GetPassParameters());
		const RenderingInfo& renderingInfo = passParameters->GetRenderingInfo();

		bool graphics = renderingInfo.colourAttachments.size();
		bool noBackwardDependencies = m_BackwardGraphicsDependentPasses.empty();
		bool noForwardDependencies = m_ForwardGraphicsDependentPasses.empty();

		if (cmdPoolType != CommandPool::QueueType::TRANSFER)
		{
			cmdBuffer->BeginDebugLabel(frameIndex, m_PassName);
		}

		//Transition resources for the pass
		{
			CommandBuffer::DependencyInfo dependencyInfo = { DependencyBit::NONE_BIT, {} };
			std::vector<Barrier2Ref>& barriers = dependencyInfo.barriers;

			//Transition input resources
			barriers.clear();
			for (Resource& resource : GetInputResources())
				barriers.push_back(TransitionResource(renderGraph, resource));
			cmdBuffer->PipelineBarrier2(frameIndex, dependencyInfo);

			//Transition output resources
			barriers.clear();
			for (Resource& resource : GetOutputResources())
				barriers.push_back(TransitionResource(renderGraph, resource));
			cmdBuffer->PipelineBarrier2(frameIndex, dependencyInfo);
		}

		if (graphics)
		{
			const uint32_t& width = renderingInfo.renderArea.extent.width;
			const uint32_t& height = renderingInfo.renderArea.extent.height;

			cmdBuffer->BeginRendering(frameIndex, renderingInfo);
			cmdBuffer->SetViewport(frameIndex, { TaskPassParameters::CreateViewport(width, height) });
			cmdBuffer->SetScissor(frameIndex, { TaskPassParameters::CreateScissor(width, height) });
		}

		cmdBuffer->BindDescriptorSets(frameIndex, passParameters->GetDescriptorSets(), 0, passParameters->GetPipeline());
		cmdBuffer->BindPipeline(frameIndex, passParameters->GetPipeline());
		m_RenderFunction(cmdBuffer, frameIndex);

		if (graphics)
		{
			cmdBuffer->EndRendering(frameIndex);
		}

		if (graphics && noForwardDependencies)
		{
			for (Resource& resource : GetOutputResources())
			{
				if (resource.IsImageView() && resource.imageView->IsSwapchainImageView())
				{
					CommandBuffer::DependencyInfo dependencyInfo = { DependencyBit::NONE_BIT, { TransitionResource(renderGraph, resource, Resource::State::PRESENT) } };
					cmdBuffer->PipelineBarrier2(frameIndex, dependencyInfo);
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

Barrier2Ref Pass::TransitionResource(RenderGraph* renderGraph, Resource& passResource, Resource::State overideNewState)
{
	const bool& srcComputeQueue = false;
	const bool& dstComputeQueue = false;

	Resource& resource = renderGraph->GetTrackedResource(passResource);
	resource.newState = passResource.newState;
	if (overideNewState != Resource::State::UNKNOWN)
		resource.newState = overideNewState;

	Resource::State& oldState = resource.oldState;
	Resource::State& newState = resource.newState;
	const Shader::StageBit& stage = passResource.stage;
	passResource = resource;

	Barrier::Type type;
	if (resource.IsImageView())
		type = Barrier::Type::IMAGE;
	else if (resource.IsBufferView())
		type = Barrier::Type::BUFFER;
	else
		return nullptr;

	auto ShaderStageToPipelineStage = [](const Shader::StageBit& stage) -> PipelineStageBit
	{
		switch (stage)
		{
		case Shader::StageBit::VERTEX_BIT:
			return PipelineStageBit::VERTEX_SHADER_BIT;
		case Shader::StageBit::TESSELLATION_CONTROL_BIT:
			return PipelineStageBit::TESSELLATION_CONTROL_SHADER_BIT;
		case Shader::StageBit::TESSELLATION_EVALUATION_BIT:
			return PipelineStageBit::TESSELLATION_EVALUATION_SHADER_BIT;
		case Shader::StageBit::GEOMETRY_BIT:
			return PipelineStageBit::GEOMETRY_SHADER_BIT;
		case Shader::StageBit::FRAGMENT_BIT:
			return PipelineStageBit::FRAGMENT_SHADER_BIT;
		case Shader::StageBit::COMPUTE_BIT:
			return PipelineStageBit::COMPUTE_SHADER_BIT;
		case Shader::StageBit::TASK_BIT:
			return PipelineStageBit::TASK_SHADER_BIT_NV;
		case Shader::StageBit::MESH_BIT:
			return PipelineStageBit::MESH_SHADER_BIT_NV;
		case Shader::StageBit::RAYGEN_BIT:
		case Shader::StageBit::ANY_HIT_BIT:
		case Shader::StageBit::CLOSEST_HIT_BIT:
		case Shader::StageBit::MISS_BIT:
		case Shader::StageBit::INTERSECTION_BIT:
		case Shader::StageBit::CALLABLE_BIT:
			return PipelineStageBit::RAY_TRACING_SHADER_BIT;
		default:
			return PipelineStageBit(0);
		}
	};

	auto GetAccessesAndLayouts = [&](Resource::State state, Shader::StageBit stage, bool src, bool computeQueue) -> std::tuple<Barrier::AccessBit, Image::Layout, PipelineStageBit>
	{
		Barrier::AccessBit accesses = Barrier::AccessBit::NONE_BIT;
		Image::Layout layout = Image::Layout::UNKNOWN;
		PipelineStageBit pipelineStage = PipelineStageBit::TOP_OF_PIPE_BIT;

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
				accesses = Barrier::AccessBit::NONE_BIT;
			layout = Image::Layout::PRESENT_SRC;
			pipelineStage = PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT;
			break;
		}
		case Resource::State::COLOUR_ATTACHMENT:
		{
			if (src)
				accesses = Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT;
			else
				accesses = Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT;
			layout = Image::Layout::COLOUR_ATTACHMENT_OPTIMAL;
			pipelineStage = PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT;
			break;
		}
		case Resource::State::DEPTH_STENCIL_ATTACHMENT:
		{
			if (src)
				accesses = Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			else
				accesses = Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			layout = Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			pipelineStage = PipelineStageBit::EARLY_FRAGMENT_TESTS_BIT | PipelineStageBit::LATE_FRAGMENT_TESTS_BIT;
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

			pipelineStage = ShaderStageToPipelineStage(stage);
			break;
		}
		case Resource::State::SHADER_READ_WRITE:
		{
			accesses = Barrier::AccessBit::SHADER_WRITE_BIT | Barrier::AccessBit::SHADER_READ_BIT;
			layout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
			pipelineStage = ShaderStageToPipelineStage(stage);
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
			layout = Image::Layout::TRANSFER_SRC_OPTIMAL;
			pipelineStage = PipelineStageBit::TRANSFER_BIT;
			break;
		}
		}

		return { accesses, layout, pipelineStage };
	};

	auto src = GetAccessesAndLayouts(oldState, stage, true, srcComputeQueue);
	auto dst = GetAccessesAndLayouts(newState, stage, false, dstComputeQueue);

	oldState = newState;

	Barrier2::CreateInfo barrierCI;
	barrierCI.type = type;
	barrierCI.srcStageMask = src._Get_rest()._Get_rest()._Myfirst._Val;
	barrierCI.srcAccess = src._Myfirst._Val;
	barrierCI.dstStageMask = dst._Get_rest()._Get_rest()._Myfirst._Val;
	barrierCI.dstAccess = dst._Myfirst._Val;
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
		barrierCI.oldLayout = src._Get_rest()._Myfirst._Val;
		barrierCI.newLayout = dst._Get_rest()._Myfirst._Val;
		barrierCI.subresourceRange = resource.imageView->GetCreateInfo().subresourceRange;
	}
	return Barrier2::Create(&barrierCI);
}