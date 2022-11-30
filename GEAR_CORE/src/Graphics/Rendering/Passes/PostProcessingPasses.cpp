#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/PostProcessingPasses.h"
#include "Graphics/Rendering/Renderer.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace passes;

using namespace miru;
using namespace base;

void PostProcessingPasses::Bloom::Prefilter(Renderer& renderer)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	Renderer::PostProcessingInfo::Bloom& bloomInfo = renderer.GetPostProcessingInfo().bloom;

	const ImageViewRef& colourImageView = renderSurface->GetColourImageView();
	const ImageRef& colourImage = colourImageView->GetCreateInfo().image;

	const uint32_t& width = colourImage->GetCreateInfo().width;
	const uint32_t& height = colourImage->GetCreateInfo().height;
	uint32_t minSize = std::min(width, height);
	uint32_t levels = std::max(static_cast<uint32_t>(log2(static_cast<double>(minSize / 8))), uint32_t(1));
	bloomInfo.width = width;
	bloomInfo.height = height;
	bloomInfo.levels = levels;

	bloomInfo.prefilterOutputImage = renderGraph.CreateImage({ Image::Type::TYPE_2D, Image::Format::R16G16B16A16_SFLOAT, width, height, 1, levels, 1, Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
		RenderGraph::ImageDesc::UsageBit::SHADER_READ_ONLY | RenderGraph::ImageDesc::UsageBit::SHADER_READ_WRITE }, "Bloom_Prefilter_Output");
	ImageViewRef prefilterOutputImageView = renderGraph.CreateImageView(bloomInfo.prefilterOutputImage, Image::Type::TYPE_2D, { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 });

	if (!bloomInfo.UB)
	{
		float zero[sizeof(UniformBufferStructures::BloomInfo)] = { 0 };
		Uniformbuffer<UniformBufferStructures::BloomInfo>::CreateInfo ubCI;
		ubCI.debugName = "GEAR_CORE_Buffer_BloomInfoUB";
		ubCI.device = renderer.GetDevice();
		ubCI.data = zero;
		bloomInfo.UB = CreateRef<Uniformbuffer<UniformBufferStructures::BloomInfo>>(&ubCI);
		bloomInfo.UB->threshold = 3.0f;
		bloomInfo.UB->upsampleScale = 2.0f;
	}
	bloomInfo.UB->SubmitData();

	Ref<TaskPassParameters> bloomPreFilterPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["BloomPreFilter"]);
	bloomPreFilterPassParameters->SetResourceView("inputImage", ResourceView(colourImageView, Resource::State::SHADER_READ_WRITE));
	bloomPreFilterPassParameters->SetResourceView("outputImage", ResourceView(prefilterOutputImageView, Resource::State::SHADER_READ_WRITE));
	bloomPreFilterPassParameters->SetResourceView("bloomInfo", ResourceView(bloomInfo.UB));
	const std::array<uint32_t, 3>& groupCount = bloomPreFilterPassParameters->GetPipeline()->GetCreateInfo().shaders[0]->GetGroupCountXYZ();

	renderGraph.AddPass("Prefilter", bloomPreFilterPassParameters, CommandPool::QueueType::COMPUTE,
		[bloomInfo, width, height, groupCount](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			CommandBuffer::DependencyInfo dependencyInfo = { DependencyBit::NONE_BIT, {} };
			Barrier2::CreateInfo bCI;
			bCI.type = Barrier::Type::BUFFER;
			bCI.srcStageMask = PipelineStageBit::COMPUTE_SHADER_BIT;
			bCI.srcAccess = Barrier::AccessBit::UNIFORM_READ_BIT;
			bCI.dstStageMask = PipelineStageBit::TRANSFER_BIT;
			bCI.dstAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
			bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.buffer = bloomInfo.UB->GetGPUBuffer();
			bCI.offset = 0;
			bCI.size = bloomInfo.UB->GetSize();
			dependencyInfo.barriers.clear();
			dependencyInfo.barriers.emplace_back(Barrier2::Create(&bCI));

			cmdBuffer->PipelineBarrier2(frameIndex, dependencyInfo);
			cmdBuffer->CopyBuffer(frameIndex, bloomInfo.UB->GetCPUBuffer(), bloomInfo.UB->GetGPUBuffer(), { { 0, 0, bloomInfo.UB->GetSize() } });
			
			bCI.srcStageMask = PipelineStageBit::TRANSFER_BIT;
			bCI.srcAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
			bCI.dstStageMask = PipelineStageBit::COMPUTE_SHADER_BIT;
			bCI.dstAccess = Barrier::AccessBit::UNIFORM_READ_BIT;
			dependencyInfo.barriers.clear();
			dependencyInfo.barriers.emplace_back(Barrier2::Create(&bCI));
			cmdBuffer->PipelineBarrier2(frameIndex, dependencyInfo);
		
			
			uint32_t _width = std::max(width / groupCount[0], uint32_t(1));
			uint32_t _height = std::max(height / groupCount[1], uint32_t(1));
			uint32_t _depth = 1 / groupCount[2];
			_width += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
			_height += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
			cmdBuffer->Dispatch(frameIndex, _width, _height, _depth);
		});
}

void PostProcessingPasses::Bloom::Downsample(Renderer& renderer) 
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	GEAR_RENDER_GRARH_EVENT_SCOPE(renderGraph, "Downsample");

	Renderer::PostProcessingInfo::Bloom& bloomInfo = renderer.GetPostProcessingInfo().bloom;
	const uint32_t& width = bloomInfo.width;
	const uint32_t& height = bloomInfo.height;
	const uint32_t& levels = bloomInfo.levels;

	if (!bloomInfo.sampler)
	{
		Sampler::CreateInfo samplerCI;
		samplerCI.debugName = "GEAR_CORE_Sampler_Bloom";
		samplerCI.device = renderer.GetDevice();;
		samplerCI.magFilter = Sampler::Filter::LINEAR;
		samplerCI.minFilter = Sampler::Filter::LINEAR;
		samplerCI.mipmapMode = Sampler::MipmapMode::NEAREST;
		samplerCI.addressModeU = Sampler::AddressMode::CLAMP_TO_EDGE;
		samplerCI.addressModeV = Sampler::AddressMode::CLAMP_TO_EDGE;
		samplerCI.addressModeW = Sampler::AddressMode::CLAMP_TO_EDGE;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.anisotropyEnable = false;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.compareEnable = false;
		samplerCI.compareOp = CompareOp::NEVER;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 0.0f;
		samplerCI.borderColour = Sampler::BorderColour::FLOAT_TRANSPARENT_BLACK;
		samplerCI.unnormalisedCoordinates = false;
		bloomInfo.sampler = Sampler::Create(&samplerCI);
	}

	bloomInfo.imageViews.clear();
	for (uint32_t i = 0; i < levels; i++)
		bloomInfo.imageViews.push_back(renderGraph.CreateImageView(bloomInfo.prefilterOutputImage, Image::Type::TYPE_2D, { Image::AspectBit::COLOUR_BIT, i, 1, 0, 1 }));

	for (uint32_t i = 0; i < (levels - 1); i++)
	{
		Ref<TaskPassParameters> bloomDownsamplePassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["BloomDownsample"]);
		bloomDownsamplePassParameters->SetResourceView("inputImageRO", ResourceView(bloomInfo.imageViews[std::min(i + 0, levels)], bloomInfo.sampler));
		bloomDownsamplePassParameters->SetResourceView("outputImage", ResourceView(bloomInfo.imageViews[std::min(i + 1, levels)], Resource::State::SHADER_READ_WRITE));
		const std::array<uint32_t, 3>& groupCount = bloomDownsamplePassParameters->GetPipeline()->GetCreateInfo().shaders[0]->GetGroupCountXYZ();

		i++;
		renderGraph.AddPass("Level: " + std::to_string(i), bloomDownsamplePassParameters, CommandPool::QueueType::COMPUTE,
			[i, width, height, groupCount](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				uint32_t _width = std::max((width >> i) / groupCount[0], uint32_t(1));
				uint32_t _height = std::max((height >> i) / groupCount[1], uint32_t(1));
				uint32_t _depth = 1 / groupCount[2];
				_width += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
				_height += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
				cmdBuffer->Dispatch(frameIndex, _width, _height, _depth);
			});
		i--;
	}
}

void PostProcessingPasses::Bloom::Upsample(Renderer& renderer) 
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	GEAR_RENDER_GRARH_EVENT_SCOPE(renderGraph, "Upsample");

	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	const ImageViewRef& colourImageView = renderSurface->GetColourImageView();
	Renderer::PostProcessingInfo::Bloom& bloomInfo = renderer.GetPostProcessingInfo().bloom;

	const uint32_t& width = bloomInfo.width;
	const uint32_t& height = bloomInfo.height;
	const uint32_t& levels = bloomInfo.levels;

	bloomInfo.imageViews[0] = colourImageView;
	for (uint32_t i = (levels - 1); i >= 1; i--)
	{
		Ref<TaskPassParameters> bloomUpsamplePassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["BloomUpsample"]);
		bloomUpsamplePassParameters->SetResourceView("inputImageRO", ResourceView(bloomInfo.imageViews[i + 0], bloomInfo.sampler));
		bloomUpsamplePassParameters->SetResourceView("outputImage", ResourceView(bloomInfo.imageViews[i - 1], Resource::State::SHADER_READ_WRITE));
		bloomUpsamplePassParameters->SetResourceView("bloomInfo", ResourceView(bloomInfo.UB));
		const std::array<uint32_t, 3>& groupCount = bloomUpsamplePassParameters->GetPipeline()->GetCreateInfo().shaders[0]->GetGroupCountXYZ();

		renderGraph.AddPass("Level: " + std::to_string(i), bloomUpsamplePassParameters, CommandPool::QueueType::COMPUTE,
			[i, width, height, groupCount](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				uint32_t _width = std::max((width >> (i - 1)) / groupCount[0], uint32_t(1));
				uint32_t _height = std::max((height >> (i - 1)) / groupCount[1], uint32_t(1));
				uint32_t _depth = 1 / groupCount[2];
				_width += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
				_height += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
				cmdBuffer->Dispatch(frameIndex, _width, _height, _depth);
			});
	}
}

void PostProcessingPasses::HDRMapping::Main(Renderer& renderer)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	const uint32_t& width = renderSurface->GetWidth();
	const uint32_t& height = renderSurface->GetHeight();

	Ref<TaskPassParameters> hdrMappingPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["HDR"]);
	hdrMappingPassParameters->SetResourceView("hdrInfo", ResourceView(renderer.GetCamera()->GetHDRInfoUB()));
	hdrMappingPassParameters->SetResourceView("hdrInput", ResourceView(renderSurface->GetColourImageView(), Resource::State::SHADER_READ_ONLY));
	hdrMappingPassParameters->AddAttachment(0, ResourceView(renderSurface->GetColourSRGBImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
	hdrMappingPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

	renderGraph.AddPass("HDR Mapping", hdrMappingPassParameters, CommandPool::QueueType::GRAPHICS,
		[](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			cmdBuffer->Draw(frameIndex, 3);
		});
}