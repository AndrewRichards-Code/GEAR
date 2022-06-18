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
		m_ColourSRGBImageCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - ColourSRGBImage";
		m_ColourSRGBImageCI.device = m_CI.pContext->GetDevice();
		m_ColourSRGBImageCI.type = Image::Type::TYPE_2D;
		m_ColourSRGBImageCI.format = SRGBFormat;
		m_ColourSRGBImageCI.width = m_CurrentWidth;
		m_ColourSRGBImageCI.height = m_CurrentHeight;
		m_ColourSRGBImageCI.depth = 1;
		m_ColourSRGBImageCI.mipLevels = 1;
		m_ColourSRGBImageCI.arrayLayers = 1;
		m_ColourSRGBImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		m_ColourSRGBImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::SAMPLED_BIT;
		m_ColourSRGBImageCI.layout = Image::Layout::UNKNOWN;
		m_ColourSRGBImageCI.size = 0;
		m_ColourSRGBImageCI.data = nullptr;
		m_ColourSRGBImageCI.allocator = m_AttachmentAllocator;
		m_ColourSRGBImage = Image::Create(&m_ColourSRGBImageCI);

		m_ColourSRGBImageViewCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - ColourSRGBImageView";
		m_ColourSRGBImageViewCI.device = m_CI.pContext->GetDevice();
		m_ColourSRGBImageViewCI.image = m_ColourSRGBImage;
		m_ColourSRGBImageViewCI.viewType = Image::Type::TYPE_2D;
		m_ColourSRGBImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_ColourSRGBImageView = ImageView::Create(&m_ColourSRGBImageViewCI);
	}

	//Depth
	{
		m_DepthImageCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - DepthImage";
		m_DepthImageCI.device = m_CI.pContext->GetDevice();
		m_DepthImageCI.type = Image::Type::TYPE_2D;
		m_DepthImageCI.format = DepthFormat;
		m_DepthImageCI.width = m_CurrentWidth;
		m_DepthImageCI.height = m_CurrentHeight;
		m_DepthImageCI.depth = 1;
		m_DepthImageCI.mipLevels = 1;
		m_DepthImageCI.arrayLayers = 1;
		m_DepthImageCI.sampleCount = m_CI.samples;
		m_DepthImageCI.usage = Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
		m_DepthImageCI.layout = Image::Layout::UNKNOWN;
		m_DepthImageCI.size = 0;
		m_DepthImageCI.data = nullptr;
		m_DepthImageCI.allocator = m_AttachmentAllocator;
		m_DepthImage = Image::Create(&m_DepthImageCI);

		m_DepthImageViewCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - DepthImageView";
		m_DepthImageViewCI.device = m_CI.pContext->GetDevice();
		m_DepthImageViewCI.image = m_DepthImage;
		m_DepthImageViewCI.viewType = Image::Type::TYPE_2D;
		m_DepthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 1 };
		m_DepthImageView = ImageView::Create(&m_DepthImageViewCI);
	}

	//MSAAColour
	if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
	{
		m_MSAAColourImageCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - MSAAColourImage";
		m_MSAAColourImageCI.device = m_CI.pContext->GetDevice();
		m_MSAAColourImageCI.type = Image::Type::TYPE_2D;
		m_MSAAColourImageCI.format = HDRFormat;
		m_MSAAColourImageCI.width = m_CurrentWidth;
		m_MSAAColourImageCI.height = m_CurrentHeight;
		m_MSAAColourImageCI.depth = 1;
		m_MSAAColourImageCI.mipLevels = 1;
		m_MSAAColourImageCI.arrayLayers = 1;
		m_MSAAColourImageCI.sampleCount = m_CI.samples;
		m_MSAAColourImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT;
		m_MSAAColourImageCI.layout = Image::Layout::UNKNOWN;
		m_MSAAColourImageCI.size = 0;
		m_MSAAColourImageCI.data = nullptr;
		m_MSAAColourImageCI.allocator = m_AttachmentAllocator;
		m_MSAAColourImage = Image::Create(&m_MSAAColourImageCI);

		m_MSAAColourImageViewCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - MSAAColourImageView";
		m_MSAAColourImageViewCI.device = m_CI.pContext->GetDevice();
		m_MSAAColourImageViewCI.image = m_MSAAColourImage;
		m_MSAAColourImageViewCI.viewType = Image::Type::TYPE_2D;
		m_MSAAColourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_MSAAColourImageView = ImageView::Create(&m_MSAAColourImageViewCI);
	}

	//Colour
	{
		m_ColourImageCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - ColourImage";
		m_ColourImageCI.device = m_CI.pContext->GetDevice();
		m_ColourImageCI.type = Image::Type::TYPE_2D;
		m_ColourImageCI.format = HDRFormat;
		m_ColourImageCI.width = m_CurrentWidth;
		m_ColourImageCI.height = m_CurrentHeight;
		m_ColourImageCI.depth = 1;
		m_ColourImageCI.mipLevels = 1;
		m_ColourImageCI.arrayLayers = 1;
		m_ColourImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		m_ColourImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::STORAGE_BIT | Image::UsageBit::INPUT_ATTACHMENT_BIT;
		m_ColourImageCI.layout = Image::Layout::UNKNOWN;
		m_ColourImageCI.size = 0;
		m_ColourImageCI.data = nullptr;
		m_ColourImageCI.allocator = m_AttachmentAllocator;
		m_ColourImage = Image::Create(&m_ColourImageCI);

		m_ColourImageViewCI.debugName = "GEAR_CORE_RenderSurface: " + m_CI.debugName + " - ColourImageView";
		m_ColourImageViewCI.device = m_CI.pContext->GetDevice();
		m_ColourImageViewCI.image = m_ColourImage;
		m_ColourImageViewCI.viewType = Image::Type::TYPE_2D;
		m_ColourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_ColourImageView = ImageView::Create(&m_ColourImageViewCI);
	}
}

void RenderSurface::Resize(uint32_t width, uint32_t height)
{
	m_CurrentWidth = width;
	m_CurrentHeight = height;

	CreateAttachments();
}
