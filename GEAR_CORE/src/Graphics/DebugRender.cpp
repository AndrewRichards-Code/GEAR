#include "gear_core_common.h"
#include "DebugRender.h"
#include "Graphics/AllocatorManager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Ref<RenderPass> DebugRender::s_DefaultRenderPass = nullptr;
void* DebugRender::s_Device = nullptr;
Image::Format DebugRender::s_ColourImageFormat = Image::Format::R8G8B8A8_UNORM;
Ref<objects::Camera> DebugRender::s_Camera = nullptr;

const Ref<RenderPass>& DebugRender::GetDefaultRenderPass()
{
	if (!s_DefaultRenderPass && s_Device)
	{
		RenderPass::CreateInfo renderPassCI;
		renderPassCI.debugName = "GEAR_CORE_RenderPass: GEAR_CORE_DebugRender: DefaultRenderPass";
		renderPassCI.device = s_Device;
		renderPassCI.attachments = 
		{ 
			//Colour
			{
				s_ColourImageFormat,
				Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
				RenderPass::AttachmentLoadOp::CLEAR,
				RenderPass::AttachmentStoreOp::STORE,
				RenderPass::AttachmentLoadOp::DONT_CARE,
				RenderPass::AttachmentStoreOp::DONT_CARE,
				Image::Layout::UNKNOWN,
				Image::Layout::SHADER_READ_ONLY_OPTIMAL
			} 
		};
		renderPassCI.subpassDescriptions = 
		{
			{ PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {}, {} }
		};
		renderPassCI.subpassDependencies = 
		{
			{
				MIRU_SUBPASS_EXTERNAL, 0, 
				PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT,
				Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT, 
				DependencyBit::NONE_BIT
			}
		};
		s_DefaultRenderPass = RenderPass::Create(&renderPassCI);
	}

	return s_DefaultRenderPass;
}

Ref<ImageView> DebugRender::CreateColourImageView()
{
	static uint32_t i = 0;

	Image::CreateInfo imageCI;
	imageCI.debugName = "GEAR_CORE_Image: GEAR_CORE_DebugRender: DefaultColourImage" + std::to_string(i);
	imageCI.device = s_Device;
	imageCI.type = Image::Type::TYPE_2D;
	imageCI.format = s_ColourImageFormat;
	imageCI.width = 256;
	imageCI.height = 256;
	imageCI.depth = 1;
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	imageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::SAMPLED_BIT;
	imageCI.layout = Image::Layout::UNKNOWN;
	imageCI.size = 0;
	imageCI.data = nullptr;
	imageCI.pAllocator = AllocatorManager::GetGPUAllocator();

	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "GEAR_CORE_ImageView: GEAR_CORE_DebugRender: DefaultColourImageView" + std::to_string(i++);
	imageViewCI.device = s_Device;
	imageViewCI.pImage = Image::Create(&imageCI);
	imageViewCI.viewType = Image::Type::TYPE_2D;
	imageViewCI.subresourceRange = {Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1};

	return ImageView::Create(&imageViewCI);
}

Ref<Framebuffer> DebugRender::CreateFramebuffer(const Ref<miru::crossplatform::ImageView>& colourImageView)
{
	static uint32_t i = 0;
	const Image::CreateInfo& colourImageCI = colourImageView->GetCreateInfo().pImage->GetCreateInfo();

	Framebuffer::CreateInfo framebufferCI;
	framebufferCI.debugName = "GEAR_CORE_Framebuffer: GEAR_CORE_DebugRender: DefaultColourFramebuffer" + std::to_string(i++);
	framebufferCI.device = s_Device;
	framebufferCI.renderPass = GetDefaultRenderPass();
	framebufferCI.attachments = { colourImageView };
	framebufferCI.width = colourImageCI.width;
	framebufferCI.height = colourImageCI.height;
	framebufferCI.layers = colourImageCI.arrayLayers;

	return Framebuffer::Create(&framebufferCI);
}

Ref<objects::Camera>& DebugRender::GetCamera()
{
	using namespace objects;
	if (!s_Camera && s_Device)
	{
		Camera::CreateInfo cameraCI;
		cameraCI.debugName = "GEAR_CORE_Camera: GEAR_CORE_DebugRender";
		cameraCI.device = s_Device;
		cameraCI.projectionType = Camera::ProjectionType::PERSPECTIVE;
		cameraCI.perspectiveParams = { mars::DegToRad(120.0), 1.0f, 0.001f, 1.0f };
		cameraCI.flipX = false;
		cameraCI.flipY = false;
		s_Camera = CreateRef<Camera>(&cameraCI);
	}

	return s_Camera;
}
