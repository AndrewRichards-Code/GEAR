#include "gear_core_common.h"
#include "RenderSurface.h"
#include "Core/StringConversion.h"
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
	m_ContextCI.api_version_major = GraphicsAPI::IsD3D12() ? 11 : 1;
	m_ContextCI.api_version_minor = 1;
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
	m_ContextCI.deviceDebugName = "GearBox_Context";
	m_Context = Context::Create(&m_ContextCI);

	m_SwapchainCI.debugName = "GearBox_Swapchain";
	m_SwapchainCI.pContext = m_Context;
	m_SwapchainCI.pWindow = m_CI.window;
	m_SwapchainCI.width = m_CurrentWidth;
	m_SwapchainCI.height = m_CurrentHeight;
	m_SwapchainCI.swapchainCount = 2;
	m_SwapchainCI.vSync = m_CI.vSync;
	m_Swapchain = Swapchain::Create(&m_SwapchainCI);

	MemoryBlock::CreateInfo dpethMBCI;
	dpethMBCI.debugName = "GearBox_MB_0_GPU_SwapchainDepthImage";
	dpethMBCI.pContext = m_Context;
	dpethMBCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_32MB;
	dpethMBCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
	m_DepthMB = MemoryBlock::Create(&dpethMBCI);

	m_DepthImageCI.debugName = "GearBox_Swapchain: DepthImage";
	m_DepthImageCI.device = m_Context->GetDevice();
	m_DepthImageCI.type = Image::Type::TYPE_2D;
	m_DepthImageCI.format = Image::Format::D32_SFLOAT;
	m_DepthImageCI.width = m_CurrentWidth;
	m_DepthImageCI.height = m_CurrentHeight;
	m_DepthImageCI.depth = 1;
	m_DepthImageCI.mipLevels = 1;
	m_DepthImageCI.arrayLayers = 1;
	m_DepthImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_DepthImageCI.usage = Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
	m_DepthImageCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
	m_DepthImageCI.size = 0;
	m_DepthImageCI.data = nullptr;
	m_DepthImageCI.pMemoryBlock = m_DepthMB;
	m_DepthImage = Image::Create(&m_DepthImageCI);

	m_DepthImageViewCI.debugName = "GearBox_Swapchain: DepthImageView";
	m_DepthImageViewCI.device = m_Context->GetDevice();
	m_DepthImageViewCI.pImage = m_DepthImage;
	m_DepthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 1 };
	m_DepthImageView = ImageView::Create(&m_DepthImageViewCI);

	m_RenderPassCI.debugName = "GearBox_RenderPass_Default";
	m_RenderPassCI.device = m_Context->GetDevice();
	m_RenderPassCI.attachments =
	{
		{m_Swapchain->m_SwapchainImages[0]->GetCreateInfo().format, Image::SampleCountBit::SAMPLE_COUNT_1_BIT, RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE,RenderPass::AttachmentLoadOp::DONT_CARE, RenderPass::AttachmentStoreOp::DONT_CARE, GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN, Image::Layout::PRESENT_SRC},
		{m_DepthImageCI.format, Image::SampleCountBit::SAMPLE_COUNT_1_BIT, RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::DONT_CARE,RenderPass::AttachmentLoadOp::DONT_CARE, RenderPass::AttachmentStoreOp::DONT_CARE, Image::Layout::UNKNOWN, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
	};
	m_RenderPassCI.subpassDescriptions =
	{
		{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {{1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}}, {}}
	};
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
		result += "VULKAN: ";
		result += std::to_string(m_ContextCI.api_version_major)
			+ "." + std::to_string(m_ContextCI.api_version_minor);
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
		result = core::to_string(&ref_cast<d3d12::Context>(m_Context)->m_PhysicalDevices.m_AdapterDescs[0].Description[0]);
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
	m_FramebufferCI.debugName = "GearBox_Framebuffer_Default";
	m_FramebufferCI.device = m_Context->GetDevice();
	m_FramebufferCI.renderPass = m_RenderPass;
	m_FramebufferCI.attachments = { m_Swapchain->m_SwapchainImageViews[0], m_DepthImageView };
	m_FramebufferCI.width = m_CurrentWidth;
	m_FramebufferCI.height = m_CurrentHeight;
	m_FramebufferCI.layers = 1;
	m_Framebuffers[0] = Framebuffer::Create(&m_FramebufferCI);
	m_FramebufferCI.attachments = { m_Swapchain->m_SwapchainImageViews[1], m_DepthImageView };
	m_Framebuffers[1] = Framebuffer::Create(&m_FramebufferCI);
}

void RenderSurface::Resize(int width, int height)
{
	m_CurrentWidth = width;
	m_CurrentHeight = height;

	m_Swapchain->Resize(static_cast<uint32_t>(m_CurrentWidth), static_cast<uint32_t>(m_CurrentHeight));
	if (width <= 3840 && height <= 2160)
	{
		m_DepthImageCI.width = m_CurrentWidth;
		m_DepthImageCI.height = m_CurrentHeight;
		m_DepthImage = Image::Create(&m_DepthImageCI);
		m_DepthImageViewCI.pImage = m_DepthImage;
		m_DepthImageView = ImageView::Create(&m_DepthImageViewCI);
	}
	CreateFramebuffer();
}
