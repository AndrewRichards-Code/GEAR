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

	if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
	{
		m_ColourImageCI.debugName = "GEAR_CORE_Swapchain: ColourImage";
		m_ColourImageCI.device = m_Context->GetDevice();
		m_ColourImageCI.type = Image::Type::TYPE_2D;
		m_ColourImageCI.format = m_Swapchain->m_SwapchainImages[0]->GetCreateInfo().format;
		m_ColourImageCI.width = m_CurrentWidth;
		m_ColourImageCI.height = m_CurrentHeight;
		m_ColourImageCI.depth = 1;
		m_ColourImageCI.mipLevels = 1;
		m_ColourImageCI.arrayLayers = 1;
		m_ColourImageCI.sampleCount = m_CI.samples;
		m_ColourImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT;
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

	m_RenderPassCI.debugName = "GEAR_CORE_RenderPass_Default";
	m_RenderPassCI.device = m_Context->GetDevice();
	if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
	{
		m_RenderPassCI.attachments =
		{
			{
				m_Swapchain->m_SwapchainImages[0]->GetCreateInfo().format,
				m_CI.samples,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
				Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
			},
			{
				m_DepthImageCI.format,
				m_CI.samples,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				Image::Layout::UNKNOWN,
				Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},
			{
				m_Swapchain->m_SwapchainImages[0]->GetCreateInfo().format,
				Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				Image::Layout::UNKNOWN,
				Image::Layout::PRESENT_SRC
			}
		};
		m_RenderPassCI.subpassDescriptions =
		{
			{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {{2, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {{1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}}, {}}
		};
	}
	else
	{
		m_RenderPassCI.attachments =
		{
			{
				m_Swapchain->m_SwapchainImages[0]->GetCreateInfo().format,
				m_CI.samples,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
				Image::Layout::PRESENT_SRC
			},
			{
				m_DepthImageCI.format,
				m_CI.samples,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				Image::Layout::UNKNOWN,
				Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			}
		};
		m_RenderPassCI.subpassDescriptions =
		{
			{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {{1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}}, {}}
		};
	}
	m_RenderPassCI.subpassDependencies =
	{
		{MIRU_SUBPASS_EXTERNAL, 0, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
		(Barrier::AccessBit)0, Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, DependencyBit::NONE_BIT}
	};
	m_RenderPass = RenderPass::Create(&m_RenderPassCI);

	CreateFramebuffer();
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

void RenderSurface::CreateFramebuffer()
{
	for (size_t i = 0; i < _countof(m_Framebuffers); i++)
	{
		m_FramebufferCI.debugName = "GEAR_CORE_Framebuffer_Default";
		m_FramebufferCI.device = m_Context->GetDevice();
		m_FramebufferCI.renderPass = m_RenderPass;

		if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
			m_FramebufferCI.attachments = { m_ColourImageView, m_DepthImageView, m_Swapchain->m_SwapchainImageViews[i] };
		else
			m_FramebufferCI.attachments = { m_Swapchain->m_SwapchainImageViews[i], m_DepthImageView };

		m_FramebufferCI.width = m_CurrentWidth;
		m_FramebufferCI.height = m_CurrentHeight;
		m_FramebufferCI.layers = 1;
		m_Framebuffers[i] = Framebuffer::Create(&m_FramebufferCI);
	}
}

void RenderSurface::Resize(int width, int height)
{
	m_CurrentWidth = width;
	m_CurrentHeight = height;

	m_Swapchain->Resize(static_cast<uint32_t>(m_CurrentWidth), static_cast<uint32_t>(m_CurrentHeight));
	if (width <= 3840 && height <= 2160)
	{
		if (m_CI.samples > Image::SampleCountBit::SAMPLE_COUNT_1_BIT)
		{
			m_ColourImageCI.width = width;
			m_ColourImageCI.height = height;
			m_ColourImage = Image::Create(&m_ColourImageCI);
			m_ColourImageViewCI.pImage = m_ColourImage;
			m_ColourImageView = ImageView::Create(&m_ColourImageViewCI);
		}

		m_DepthImageCI.width = m_CurrentWidth;
		m_DepthImageCI.height = m_CurrentHeight;
		m_DepthImage = Image::Create(&m_DepthImageCI);
		m_DepthImageViewCI.pImage = m_DepthImage;
		m_DepthImageView = ImageView::Create(&m_DepthImageViewCI);
	}
	CreateFramebuffer();
}
