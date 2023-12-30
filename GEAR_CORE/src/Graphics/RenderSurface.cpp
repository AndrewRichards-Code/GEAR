#include "gear_core_common.h"
#include "Graphics/RenderSurface.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace base;

RenderSurface::RenderSurface(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	Allocator::CreateInfo attachmentAllocatorCI;
	attachmentAllocatorCI.debugName = "GEAR_CORE_GPU_ALLOCATOR_Attachments";
	attachmentAllocatorCI.context = m_CI.pContext;
	attachmentAllocatorCI.blockSize = Allocator::BlockSize::BLOCK_SIZE_64MB;
	attachmentAllocatorCI.properties = Allocator::PropertiesBit::DEVICE_LOCAL_BIT;
	m_AttachmentAllocator = Allocator::Create(&attachmentAllocatorCI);

	m_CurrentWidth = m_CI.width;
	m_CurrentHeight = m_CI.height;

	CreateAttachments();
}

RenderSurface::~RenderSurface()
{
	m_CI.pContext->DeviceWaitIdle();
}

void RenderSurface::CreateAttachments()
{
	//ColourSRGB
	{
		Image::CreateInfo colourSRGBImageCI;
		colourSRGBImageCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - ColourSRGBImage";
		colourSRGBImageCI.device = m_CI.pContext->GetDevice();
		colourSRGBImageCI.type = Image::Type::TYPE_2D;
		colourSRGBImageCI.format = SRGBFormat;
		colourSRGBImageCI.width = m_CurrentWidth;
		colourSRGBImageCI.height = m_CurrentHeight;
		colourSRGBImageCI.depth = 1;
		colourSRGBImageCI.mipLevels = 1;
		colourSRGBImageCI.arrayLayers = 1;
		colourSRGBImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		colourSRGBImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::SAMPLED_BIT;
		colourSRGBImageCI.layout = Image::Layout::UNKNOWN;
		colourSRGBImageCI.size = 0;
		colourSRGBImageCI.data = nullptr;
		colourSRGBImageCI.allocator = m_AttachmentAllocator;
		colourSRGBImageCI.externalImage = nullptr;
		m_ColourSRGBImage = Image::Create(&colourSRGBImageCI);

		ImageView::CreateInfo colourSRGBImageViewCI;
		colourSRGBImageViewCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - ColourSRGBImageView";
		colourSRGBImageViewCI.device = m_CI.pContext->GetDevice();
		colourSRGBImageViewCI.image = m_ColourSRGBImage;
		colourSRGBImageViewCI.viewType = Image::Type::TYPE_2D;
		colourSRGBImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_ColourSRGBImageView = ImageView::Create(&colourSRGBImageViewCI);
	}

	//Depth
	{
		Image::CreateInfo depthImageCI;
		depthImageCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - DepthImage";
		depthImageCI.device = m_CI.pContext->GetDevice();
		depthImageCI.type = Image::Type::TYPE_2D;
		depthImageCI.format = DepthFormat;
		depthImageCI.width = m_CurrentWidth;
		depthImageCI.height = m_CurrentHeight;
		depthImageCI.depth = 1;
		depthImageCI.mipLevels = 1;
		depthImageCI.arrayLayers = 1;
		depthImageCI.sampleCount = m_CI.samples;
		depthImageCI.usage = Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImageCI.layout = Image::Layout::UNKNOWN;
		depthImageCI.size = 0;
		depthImageCI.data = nullptr;
		depthImageCI.allocator = m_AttachmentAllocator;
		depthImageCI.externalImage = nullptr;
		m_DepthImage = Image::Create(&depthImageCI);

		ImageView::CreateInfo depthImageViewCI;
		depthImageViewCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - DepthImageView";
		depthImageViewCI.device = m_CI.pContext->GetDevice();
		depthImageViewCI.image = m_DepthImage;
		depthImageViewCI.viewType = Image::Type::TYPE_2D;
		depthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 1 };
		m_DepthImageView = ImageView::Create(&depthImageViewCI);
	}

	//MSAAColour
	if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
	{
		Image::CreateInfo MSAAColourImageCI;
		MSAAColourImageCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - MSAAColourImage";
		MSAAColourImageCI.device = m_CI.pContext->GetDevice();
		MSAAColourImageCI.type = Image::Type::TYPE_2D;
		MSAAColourImageCI.format = HDRFormat;
		MSAAColourImageCI.width = m_CurrentWidth;
		MSAAColourImageCI.height = m_CurrentHeight;
		MSAAColourImageCI.depth = 1;
		MSAAColourImageCI.mipLevels = 1;
		MSAAColourImageCI.arrayLayers = 1;
		MSAAColourImageCI.sampleCount = m_CI.samples;
		MSAAColourImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT;
		MSAAColourImageCI.layout = Image::Layout::UNKNOWN;
		MSAAColourImageCI.size = 0;
		MSAAColourImageCI.data = nullptr;
		MSAAColourImageCI.allocator = m_AttachmentAllocator;
		MSAAColourImageCI.externalImage = nullptr;
		m_MSAAColourImage = Image::Create(&MSAAColourImageCI);

		ImageView::CreateInfo MSAAColourImageViewCI;
		MSAAColourImageViewCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - MSAAColourImageView";
		MSAAColourImageViewCI.device = m_CI.pContext->GetDevice();
		MSAAColourImageViewCI.image = m_MSAAColourImage;
		MSAAColourImageViewCI.viewType = Image::Type::TYPE_2D;
		MSAAColourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_MSAAColourImageView = ImageView::Create(&MSAAColourImageViewCI);
	}

	//Colour
	{
		Image::CreateInfo colourImageCI;
		colourImageCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - ColourImage";
		colourImageCI.device = m_CI.pContext->GetDevice();
		colourImageCI.type = Image::Type::TYPE_2D;
		colourImageCI.format = HDRFormat;
		colourImageCI.width = m_CurrentWidth;
		colourImageCI.height = m_CurrentHeight;
		colourImageCI.depth = 1;
		colourImageCI.mipLevels = 1;
		colourImageCI.arrayLayers = 1;
		colourImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		colourImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::STORAGE_BIT | Image::UsageBit::SAMPLED_BIT;
		colourImageCI.layout = Image::Layout::UNKNOWN;
		colourImageCI.size = 0;
		colourImageCI.data = nullptr;
		colourImageCI.allocator = m_AttachmentAllocator;
		colourImageCI.externalImage = nullptr;
		m_ColourImage = Image::Create(&colourImageCI);

		ImageView::CreateInfo colourImageViewCI;
		colourImageViewCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - ColourImageView";
		colourImageViewCI.device = m_CI.pContext->GetDevice();
		colourImageViewCI.image = m_ColourImage;
		colourImageViewCI.viewType = Image::Type::TYPE_2D;
		colourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_ColourImageView = ImageView::Create(&colourImageViewCI);
	}
}

void RenderSurface::Resize(uint32_t width, uint32_t height)
{
	m_CurrentWidth = width;
	m_CurrentHeight = height;

	CreateAttachments();
}
