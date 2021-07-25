#include "gear_core_common.h"
#include "RenderSurface.h"
#include "ARC/src/StringConversion.h"
#include "directx12/D3D12Context.h"
#include "vulkan/VKContext.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

RenderSurface::RenderSurface(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_CurrentWidth = m_CI.width;
	m_CurrentHeight = m_CI.height;

	GraphicsAPI::SetAPI(m_CI.api);
	GraphicsAPI::AllowSetName();
	GraphicsAPI::LoadGraphicsDebugger(m_CI.graphicsDebugger);

	m_ContextCI.applicationName = m_CI.debugName.c_str();
	m_ContextCI.api_version_major = GraphicsAPI::IsD3D12() ? 12 : 1;
	m_ContextCI.api_version_minor = GraphicsAPI::IsD3D12() ? 1 : 2;
#ifdef _DEBUG
	m_ContextCI.instanceLayers = { "VK_LAYER_KHRONOS_validation" };
	m_ContextCI.instanceExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
	m_ContextCI.deviceLayers = { "VK_LAYER_KHRONOS_validation" };
	m_ContextCI.deviceExtensions = { "VK_KHR_swapchain" };
#else
	m_ContextCI.instanceLayers = {};
	m_ContextCI.instanceExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
	m_ContextCI.deviceLayers = {};
	m_ContextCI.deviceExtensions = { "VK_KHR_swapchain" };
#endif
	m_ContextCI.deviceDebugName = "GEAR_CORE_Context";
	m_Context = Context::Create(&m_ContextCI);

	m_SwapchainCI.debugName = "GEAR_CORE_Swapchain";
	m_SwapchainCI.pContext = m_Context;
	m_SwapchainCI.pWindow = m_CI.window;
	m_SwapchainCI.width = m_CurrentWidth;
	m_SwapchainCI.height = m_CurrentHeight;
	m_SwapchainCI.swapchainCount = 2;
	m_SwapchainCI.vSync = m_CI.vSync;
	m_SwapchainCI.bpcColourSpace = m_CI.bpcColourSpace;
	m_Swapchain = Swapchain::Create(&m_SwapchainCI);

	Allocator::CreateInfo attachmentAllocatorCI;
	attachmentAllocatorCI.debugName = "GEAR_CORE_GPU_ALLOCATOR_Attachments";
	attachmentAllocatorCI.pContext = m_Context;
	attachmentAllocatorCI.blockSize = Allocator::BlockSize::BLOCK_SIZE_64MB;
	attachmentAllocatorCI.properties = Allocator::PropertiesBit::DEVICE_LOCAL_BIT;
	m_AttachmentAllocator = Allocator::Create(&attachmentAllocatorCI);

	CreateAttachments();

	CreateMainRenderPass();
	CreateHDRRenderPass();

	CreateMainFramebuffer();
	CreateHDRFramebuffer();
}

RenderSurface::~RenderSurface()
{
	m_Context->DeviceWaitIdle();
}

std::string RenderSurface::GetGraphicsAPIVersion() const
{
	std::string result("");
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::UNKNOWN:
	{
		result += "UNKNOWN"; break;
	}
	case GraphicsAPI::API::D3D12:
	{
		result += "D3D12: ";
		result += "D3D_FEATURE_LEVEL_" + std::to_string(m_ContextCI.api_version_major)
			+ "_" + std::to_string(m_ContextCI.api_version_minor);
		break;
	}
	case GraphicsAPI::API::VULKAN:
	{
		uint32_t version = ref_cast<vulkan::Context>(m_Context)->m_PhysicalDevices.m_PhysicalDeviceProperties[0].apiVersion;

		result += "VULKAN: ";
		result += std::to_string(m_ContextCI.api_version_major)
			+ "." + std::to_string(m_ContextCI.api_version_minor)
			+ "." + std::to_string(VK_VERSION_PATCH(version));
		break;
	}
	}

	return result;
}

std::string RenderSurface::GetDeviceName() const
{
	std::string result;
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::UNKNOWN:
	{
		result += "UNKNOWN"; break;
	}
	case GraphicsAPI::API::D3D12:
	{
		result = arc::ToString(&ref_cast<d3d12::Context>(m_Context)->m_PhysicalDevices.m_AdapterDescs[0].Description[0]);
		break;
	}
	case GraphicsAPI::API::VULKAN:
	{
		result = &ref_cast<vulkan::Context>(m_Context)->m_PhysicalDevices.m_PhysicalDeviceProperties[0].deviceName[0];
		break;
	}
	}

	return result;
}

void RenderSurface::CreateAttachments()
{
	//Depth
	{
		m_DepthImageCI.debugName = "GEAR_CORE_Swapchain: DepthImage";
		m_DepthImageCI.device = m_Context->GetDevice();
		m_DepthImageCI.type = Image::Type::TYPE_2D;
		m_DepthImageCI.format = Image::Format::D32_SFLOAT;
		m_DepthImageCI.width = m_CurrentWidth;
		m_DepthImageCI.height = m_CurrentHeight;
		m_DepthImageCI.depth = 1;
		m_DepthImageCI.mipLevels = 1;
		m_DepthImageCI.arrayLayers = 1;
		m_DepthImageCI.sampleCount = m_CI.samples;
		m_DepthImageCI.usage = Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
		m_DepthImageCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
		m_DepthImageCI.size = 0;
		m_DepthImageCI.data = nullptr;
		m_DepthImageCI.pAllocator = m_AttachmentAllocator;
		m_DepthImage = Image::Create(&m_DepthImageCI);

		m_DepthImageViewCI.debugName = "GEAR_CORE_Swapchain: DepthImageView";
		m_DepthImageViewCI.device = m_Context->GetDevice();
		m_DepthImageViewCI.pImage = m_DepthImage;
		m_DepthImageViewCI.viewType = Image::Type::TYPE_2D;
		m_DepthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 1 };
		m_DepthImageView = ImageView::Create(&m_DepthImageViewCI);
	}

	//MSAAColour
	if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
	{
		m_MSAAColourImageCI.debugName = "GEAR_CORE_Swapchain: MSAAColourImage";
		m_MSAAColourImageCI.device = m_Context->GetDevice();
		m_MSAAColourImageCI.type = Image::Type::TYPE_2D;
		m_MSAAColourImageCI.format = m_HDRFormat;
		m_MSAAColourImageCI.width = m_CurrentWidth;
		m_MSAAColourImageCI.height = m_CurrentHeight;
		m_MSAAColourImageCI.depth = 1;
		m_MSAAColourImageCI.mipLevels = 1;
		m_MSAAColourImageCI.arrayLayers = 1;
		m_MSAAColourImageCI.sampleCount = m_CI.samples;
		m_MSAAColourImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT;
		m_MSAAColourImageCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::COLOUR_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
		m_MSAAColourImageCI.size = 0;
		m_MSAAColourImageCI.data = nullptr;
		m_MSAAColourImageCI.pAllocator = m_AttachmentAllocator;
		m_MSAAColourImage = Image::Create(&m_MSAAColourImageCI);

		m_MSAAColourImageViewCI.debugName = "GEAR_CORE_Swapchain: MSAAColourImageView";
		m_MSAAColourImageViewCI.device = m_Context->GetDevice();
		m_MSAAColourImageViewCI.pImage = m_MSAAColourImage;
		m_MSAAColourImageViewCI.viewType = Image::Type::TYPE_2D;
		m_MSAAColourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_MSAAColourImageView = ImageView::Create(&m_MSAAColourImageViewCI);
	}

	//Colour
	{
		m_ColourImageCI.debugName = "GEAR_CORE_Swapchain: ColourImage";
		m_ColourImageCI.device = m_Context->GetDevice();
		m_ColourImageCI.type = Image::Type::TYPE_2D;
		m_ColourImageCI.format = m_HDRFormat;
		m_ColourImageCI.width = m_CurrentWidth;
		m_ColourImageCI.height = m_CurrentHeight;
		m_ColourImageCI.depth = 1;
		m_ColourImageCI.mipLevels = 1;
		m_ColourImageCI.arrayLayers = 1;
		m_ColourImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		m_ColourImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::STORAGE_BIT | Image::UsageBit::INPUT_ATTACHMENT_BIT;
		m_ColourImageCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::COLOUR_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
		m_ColourImageCI.size = 0;
		m_ColourImageCI.data = nullptr;
		m_ColourImageCI.pAllocator = m_AttachmentAllocator;
		m_ColourImage = Image::Create(&m_ColourImageCI);

		m_ColourImageViewCI.debugName = "GEAR_CORE_Swapchain: ColourImageView";
		m_ColourImageViewCI.device = m_Context->GetDevice();
		m_ColourImageViewCI.pImage = m_ColourImage;
		m_ColourImageViewCI.viewType = Image::Type::TYPE_2D;
		m_ColourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_ColourImageView = ImageView::Create(&m_ColourImageViewCI);
	}

	//MSAAEmissive
	if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
	{
		m_MSAAEmissiveImageCI.debugName = "GEAR_CORE_Swapchain: MSAAEmissiveImage";
		m_MSAAEmissiveImageCI.device = m_Context->GetDevice();
		m_MSAAEmissiveImageCI.type = Image::Type::TYPE_2D;
		m_MSAAEmissiveImageCI.format = m_HDRFormat;
		m_MSAAEmissiveImageCI.width = m_CurrentWidth;
		m_MSAAEmissiveImageCI.height = m_CurrentHeight;
		m_MSAAEmissiveImageCI.depth = 1;
		m_MSAAEmissiveImageCI.mipLevels = 1;
		m_MSAAEmissiveImageCI.arrayLayers = 1;
		m_MSAAEmissiveImageCI.sampleCount = m_CI.samples;
		m_MSAAEmissiveImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT;
		m_MSAAEmissiveImageCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::COLOUR_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
		m_MSAAEmissiveImageCI.size = 0;
		m_MSAAEmissiveImageCI.data = nullptr;
		m_MSAAEmissiveImageCI.pAllocator = m_AttachmentAllocator;
		m_MSAAEmissiveImage = Image::Create(&m_MSAAEmissiveImageCI);

		m_MSAAEmissiveImageViewCI.debugName = "GEAR_CORE_Swapchain: MSAAEmissiveImageView";
		m_MSAAEmissiveImageViewCI.device = m_Context->GetDevice();
		m_MSAAEmissiveImageViewCI.pImage = m_MSAAEmissiveImage;
		m_MSAAEmissiveImageViewCI.viewType = Image::Type::TYPE_2D;
		m_MSAAEmissiveImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_MSAAEmissiveImageView = ImageView::Create(&m_MSAAEmissiveImageViewCI);
	}

	//Emissive
	{
		m_EmissiveImageCI.debugName = "GEAR_CORE_Swapchain: EmissiveImage";
		m_EmissiveImageCI.device = m_Context->GetDevice();
		m_EmissiveImageCI.type = Image::Type::TYPE_2D;
		m_EmissiveImageCI.format = m_HDRFormat;
		m_EmissiveImageCI.width = m_CurrentWidth;
		m_EmissiveImageCI.height = m_CurrentHeight;
		m_EmissiveImageCI.depth = 1;
		m_EmissiveImageCI.mipLevels = 1;
		m_EmissiveImageCI.arrayLayers = 1;
		m_EmissiveImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		m_EmissiveImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::STORAGE_BIT | Image::UsageBit::INPUT_ATTACHMENT_BIT;
		m_EmissiveImageCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::COLOUR_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
		m_EmissiveImageCI.size = 0;
		m_EmissiveImageCI.data = nullptr;
		m_EmissiveImageCI.pAllocator = m_AttachmentAllocator;
		m_EmissiveImage = Image::Create(&m_EmissiveImageCI);

		m_EmissiveImageViewCI.debugName = "GEAR_CORE_Swapchain: EmissiveImageView";
		m_EmissiveImageViewCI.device = m_Context->GetDevice();
		m_EmissiveImageViewCI.pImage = m_EmissiveImage;
		m_EmissiveImageViewCI.viewType = Image::Type::TYPE_2D;
		m_EmissiveImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		m_EmissiveImageView = ImageView::Create(&m_EmissiveImageViewCI);
	}
}

void RenderSurface::CreateMainRenderPass()
{
	m_MainRenderPassCI.debugName = "GEAR_CORE_MainRenderPass";
	m_MainRenderPassCI.device = m_Context->GetDevice();

	if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
	{
		m_MainRenderPassCI.attachments =
		{
			{	//0 - Depth
				m_DepthImageCI.format,
				m_CI.samples,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				Image::Layout::UNKNOWN,
				Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},
			{	//1 - MSAAColour
				m_HDRFormat,
				m_CI.samples,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
				Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
			},
			{	//2 - Colour
				m_HDRFormat,
				Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
				Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
			},
			{	//3 - MSAAEmissive
				m_HDRFormat,
				m_CI.samples,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
				Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
			},
			{	//4 - Emissive
				m_HDRFormat,
				Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
				Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
			},
		};
		m_MainRenderPassCI.subpassDescriptions =
		{
			{
				PipelineType::GRAPHICS,
				{},
				{{1, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL},{3, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}},
				{{2, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL},{4, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}},
				{{0, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}},
				{}
			}
		};
	}
	else
	{
		m_MainRenderPassCI.attachments =
		{
			{	//0 - Depth
				m_DepthImageCI.format,
				m_CI.samples,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				Image::Layout::UNKNOWN,
				Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},
			{	//1 - Colour
				m_HDRFormat,
				Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
				Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
			},
			{	//2 - Emissive
				m_HDRFormat,
				Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
				Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
			},
		};
		m_MainRenderPassCI.subpassDescriptions =
		{
			{
				PipelineType::GRAPHICS,
				{},
				{{1, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL},{2, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}},
				{},
				{{0, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}},
				{}
			}
		};
	}

	m_MainRenderPassCI.subpassDependencies =
	{
		{
			MIRU_SUBPASS_EXTERNAL,
			0,
			PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
			(Barrier::AccessBit)0,
			Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT,
			DependencyBit::NONE_BIT
		}
	};
	m_MainRenderPass = RenderPass::Create(&m_MainRenderPassCI);
}

void RenderSurface::CreateHDRRenderPass()
{
	m_HDRRenderPassCI.debugName = "GEAR_CORE_HDRRenderPass";
	m_HDRRenderPassCI.device = m_Context->GetDevice();
	m_HDRRenderPassCI.attachments =
	{
		{	//0 - Swapchain
			m_Swapchain->m_SwapchainImages[0]->GetCreateInfo().format,
			Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
			RenderPass::AttachmentLoadOp::CLEAR,
			RenderPass::AttachmentStoreOp::STORE,
			RenderPass::AttachmentLoadOp::DONT_CARE,
			RenderPass::AttachmentStoreOp::DONT_CARE,
			Image::Layout::UNKNOWN,
			Image::Layout::PRESENT_SRC
		},
		{	//1 - Colour
			m_HDRFormat,
			Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
			RenderPass::AttachmentLoadOp::LOAD,
			RenderPass::AttachmentStoreOp::DONT_CARE,
			RenderPass::AttachmentLoadOp::DONT_CARE,
			RenderPass::AttachmentStoreOp::DONT_CARE,
			Image::Layout::COLOUR_ATTACHMENT_OPTIMAL,
			Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
		},
		{	//2 - Emissive
			m_HDRFormat,
			Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
			RenderPass::AttachmentLoadOp::LOAD,
			RenderPass::AttachmentStoreOp::DONT_CARE,
			RenderPass::AttachmentLoadOp::DONT_CARE,
			RenderPass::AttachmentStoreOp::DONT_CARE,
			Image::Layout::COLOUR_ATTACHMENT_OPTIMAL,
			Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
		},
	};
	m_HDRRenderPassCI.subpassDescriptions =
	{
		{
			PipelineType::GRAPHICS,
			{{1, Image::Layout::SHADER_READ_ONLY_OPTIMAL},{2, Image::Layout::SHADER_READ_ONLY_OPTIMAL}},
			{{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}},
			{},
			{},
			{}
		}
	};
	m_HDRRenderPassCI.subpassDependencies =
	{
		{
			MIRU_SUBPASS_EXTERNAL,
			0,
			PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
			(Barrier::AccessBit)0,
			Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT,
			DependencyBit::NONE_BIT
		}
	};
	m_HDRRenderPass = RenderPass::Create(&m_HDRRenderPassCI);
}

void RenderSurface::CreateMainFramebuffer()
{
	for (size_t i = 0; i < _countof(m_MainFramebuffers); i++)
	{
		m_MainFramebufferCI.debugName = "GEAR_CORE_MainFramebuffer";
		m_MainFramebufferCI.device = m_Context->GetDevice();
		m_MainFramebufferCI.renderPass = m_MainRenderPass;

		if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
			m_MainFramebufferCI.attachments = { m_DepthImageView, m_MSAAColourImageView, m_ColourImageView, m_MSAAEmissiveImageView, m_EmissiveImageView };
		else
			m_MainFramebufferCI.attachments = { m_DepthImageView, m_ColourImageView, m_EmissiveImageView };

		m_MainFramebufferCI.width = m_CurrentWidth;
		m_MainFramebufferCI.height = m_CurrentHeight;
		m_MainFramebufferCI.layers = 1;
		m_MainFramebuffers[i] = Framebuffer::Create(&m_MainFramebufferCI);
	}
}

void RenderSurface::CreateHDRFramebuffer()
{
	for (size_t i = 0; i < _countof(m_HDRFramebuffers); i++)
	{
		m_HDRFramebufferCI.debugName = "GEAR_CORE_HDRFramebuffer";
		m_HDRFramebufferCI.device = m_Context->GetDevice();
		m_HDRFramebufferCI.renderPass = m_HDRRenderPass;
		m_HDRFramebufferCI.attachments = { m_Swapchain->m_SwapchainImageViews[i], m_ColourImageView, m_EmissiveImageView };
		m_HDRFramebufferCI.width = m_CurrentWidth;
		m_HDRFramebufferCI.height = m_CurrentHeight;
		m_HDRFramebufferCI.layers = 1;
		m_HDRFramebuffers[i] = Framebuffer::Create(&m_HDRFramebufferCI);
	}
}

void RenderSurface::Resize(int width, int height)
{
	m_CurrentWidth = width;
	m_CurrentHeight = height;

	m_Swapchain->Resize(static_cast<uint32_t>(m_CurrentWidth), static_cast<uint32_t>(m_CurrentHeight));
	if (width > 3840 || height > 2160)
	{
		MIRU_ASSERT(true, "Resize event's dimensions are greater then 4K.");
	}
	CreateAttachments();
	CreateMainFramebuffer();
	CreateHDRFramebuffer();
}
