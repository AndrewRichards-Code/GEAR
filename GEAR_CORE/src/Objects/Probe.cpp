#include "gear_core_common.h"
#include "Probe.h"

using namespace gear;
using namespace objects;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

using namespace mars;

Probe::Probe(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseUB();

	if (m_CI.captureType == CaptureType::REFLECTION)
		CreateTexture(m_ColourTexture, m_ColourTextureCI);

	CreateTexture(m_DepthTexture, m_DepthTextureCI, false);

	CreateRenderPass();
	CreateFramebuffer();
	CreateRenderPipeline();
}

Probe::~Probe()
{
}

void Probe::Update(const Transform& transform)
{
	if (CreateInfoHasChanged(&m_CI))
	{
		uint64_t newHash = m_CreateInfoHash;
		*this = Probe(&m_CI);
		m_CreateInfoHash = newHash; //Set the Hash value from the previous instance of the Mesh.
	}
	if (TransformHasChanged(transform))
	{
		//Projection
		{
			if (m_CI.directionType == DirectionType::OMNI)
				m_CI.projectionType = Camera::ProjectionType::PERSPECTIVE;

			const float& aspectRatio = static_cast<float>(m_CI.imageWidth) / static_cast<float>(m_CI.imageHeight);
			if (m_CI.projectionType == Camera::ProjectionType::PERSPECTIVE)
			{
				m_UB->proj = float4x4::Perspective(
					m_CI.directionType == DirectionType::OMNI ? DegToRad(90.0) : m_CI.perspectiveHorizonalFOV,
					m_CI.directionType == DirectionType::OMNI ? 1.0f : aspectRatio,
					m_CI.zNear,
					m_CI.zFar);
			}
			else
			{
				m_UB->proj = float4x4::Orthographic(
					-1.0f,
					+1.0f,
					-aspectRatio,
					+aspectRatio,
					m_CI.zNear,
					m_CI.zFar);
			}
			if (miru::crossplatform::GraphicsAPI::IsVulkan())
				m_UB->proj.f *= -1;
		}

		//View
		{
			if (m_CI.directionType == DirectionType::MONO)
			{
				m_UB->view[0] = TransformToMatrix4(transform).Inverse();
			}
			else
			{
				for (uint32_t faceIndex = 0; faceIndex < 6; faceIndex++)
				{
					m_UB->view[faceIndex] = Camera::GetCubemapFaceViewMatrix(faceIndex, transform.translation);
				}
			}
		}

		//Position
		{
			m_UB->position = float4(transform.translation, 1.0f);
		}

		m_UB->SubmitData();
	}
}

bool Probe::CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo)
{
	const CreateInfo& CI = *reinterpret_cast<const CreateInfo*>(pCreateInfo);
	uint64_t newHash = 0;
	newHash ^= core::GetHash(CI.directionType);
	newHash ^= core::GetHash(CI.captureType);
	newHash ^= core::GetHash(CI.imageWidth);
	newHash ^= core::GetHash(CI.imageHeight);
	newHash ^= core::GetHash(CI.projectionType);
	newHash ^= core::GetHash(CI.perspectiveHorizonalFOV);
	newHash ^= core::GetHash(CI.zNear);
	newHash ^= core::GetHash(CI.zFar);
	return CompareCreateInfoHash(newHash);
}

void Probe::CreateTexture(Ref<Texture>& texture, Texture::CreateInfo& textureCI, bool colour)
{
	//TODO: Add mips and multisampling 
	textureCI.debugName = (colour ? "GEAR_CORE_Texture_Colour: " : "GEAR_CORE_Texture_Depth: ") + m_CI.debugName;
	textureCI.device = m_CI.device;
	textureCI.dataType = Texture::DataType::DATA;
	textureCI.data.data = nullptr;
	textureCI.data.size = 0;
	textureCI.data.width = m_CI.imageWidth;
	textureCI.data.height = m_CI.directionType == DirectionType::MONO ? m_CI.imageHeight : m_CI.imageWidth;
	textureCI.data.depth = 1;
	textureCI.mipLevels = 1;
	textureCI.arrayLayers = m_CI.directionType == DirectionType::OMNI ? 6 : 1;
	textureCI.type = m_CI.directionType == DirectionType::OMNI ? Image::Type::TYPE_CUBE : Image::Type::TYPE_2D;
	textureCI.format = m_CI.captureType == CaptureType::REFLECTION ? Image::Format::R32G32B32A32_SFLOAT : Image::Format::D32_SFLOAT;
	textureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	textureCI.usage = Image::UsageBit::SAMPLED_BIT | (m_CI.captureType == CaptureType::REFLECTION ? Image::UsageBit::COLOUR_ATTACHMENT_BIT : Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT);
	textureCI.generateMipMaps = false;
	textureCI.gammaSpace = GammaSpace::LINEAR;
	texture = CreateRef<Texture>(&textureCI);
}

void Probe::CreateRenderPass()
{
	m_RenderPassCI.debugName = "GEAR_CORE_RenderPass: " + m_CI.debugName;
	m_RenderPassCI.device = m_CI.device;
	if (m_CI.captureType == CaptureType::REFLECTION)
	{
		RenderPass::AttachmentDescription attachment;
		attachment.format = m_ColourTextureCI.format;
		attachment.samples = m_ColourTextureCI.samples;
		attachment.loadOp = RenderPass::AttachmentLoadOp::CLEAR;
		attachment.storeOp = RenderPass::AttachmentStoreOp::STORE;
		attachment.stencilLoadOp = RenderPass::AttachmentLoadOp::DONT_CARE;
		attachment.stencilStoreOp = RenderPass::AttachmentStoreOp::DONT_CARE;
		attachment.initialLayout = Image::Layout::UNKNOWN;
		attachment.finalLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		m_RenderPassCI.attachments.push_back(attachment);
	}
	RenderPass::AttachmentDescription attachment;
	attachment.format = m_DepthTextureCI.format;
	attachment.samples = m_DepthTextureCI.samples;
	attachment.loadOp = RenderPass::AttachmentLoadOp::CLEAR;
	attachment.storeOp = RenderPass::AttachmentStoreOp::STORE;
	attachment.stencilLoadOp = RenderPass::AttachmentLoadOp::DONT_CARE;
	attachment.stencilStoreOp = RenderPass::AttachmentStoreOp::DONT_CARE;
	attachment.initialLayout = Image::Layout::UNKNOWN;
	attachment.finalLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
	m_RenderPassCI.attachments.push_back(attachment);

	RenderPass::SubpassDescription description;
	if (m_CI.captureType == CaptureType::REFLECTION)
	{
		description.pipelineType = PipelineType::GRAPHICS;
		description.inputAttachments = {};
		description.colourAttachments = { { 0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL } };
		description.resolveAttachments = {};
		description.depthStencilAttachment = { { 1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL} };
		description.preseverseAttachments = {};
	}
	else
	{
		description.pipelineType = PipelineType::GRAPHICS;
		description.inputAttachments = {};
		description.colourAttachments = {};
		description.resolveAttachments = {};
		description.depthStencilAttachment = { { 0, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL} };
		description.preseverseAttachments = {};
	}
	m_RenderPassCI.subpassDescriptions.push_back(description);

	RenderPass::SubpassDependency dependency;
	dependency.srcSubpass = MIRU_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStage = PipelineStageBit::TRANSFER_BIT;
	dependency.dstStage = PipelineStageBit::FRAGMENT_SHADER_BIT;
	dependency.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
	dependency.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
	dependency.dependencies = DependencyBit::NONE_BIT;
	m_RenderPassCI.subpassDependencies.push_back(dependency);

	m_RenderPass = RenderPass::Create(&m_RenderPassCI);
}

void Probe::CreateFramebuffer()
{
	m_FramebufferCI.debugName = "GEAR_CORE_Framebuffer: " + m_CI.debugName;
	m_FramebufferCI.device = m_CI.device;
	m_FramebufferCI.renderPass = m_RenderPass;
	if (m_CI.captureType == CaptureType::REFLECTION)
		m_FramebufferCI.attachments = { m_ColourTexture->GetTextureImageView(), m_DepthTexture->GetTextureImageView() };
	else
		m_FramebufferCI.attachments = { m_DepthTexture->GetTextureImageView() };
	m_FramebufferCI.width = m_DepthTextureCI.data.width;
	m_FramebufferCI.height = m_DepthTextureCI.data.height;
	m_FramebufferCI.layers = m_CI.directionType == DirectionType::OMNI ? 6 : 1;
	m_Framebuffer = Framebuffer::Create(&m_FramebufferCI);
}

void Probe::CreateRenderPipeline()
{
	m_ShadowRenderPipelineLI.device = m_CI.device;
	m_ShadowRenderPipelineLI.filepath = "res/pipelines/Shadow.grpf";
	m_ShadowRenderPipelineLI.viewportWidth = static_cast<float>(m_FramebufferCI.width);
	m_ShadowRenderPipelineLI.viewportHeight = static_cast<float>(m_FramebufferCI.height);
	m_ShadowRenderPipelineLI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_ShadowRenderPipelineLI.renderPass = m_RenderPass;
	m_ShadowRenderPipelineLI.subpassIndex = 0;
	m_ShadowRenderPipeline = CreateRef<RenderPipeline>(&m_ShadowRenderPipelineLI);
}

void Probe::InitialiseUB()
{
	if (!m_UB)
	{
		float zero0[sizeof(ProbeInfoUB)] = { 0 };

		Uniformbuffer<ProbeInfoUB>::CreateInfo ubCI;
		ubCI.debugName = "GEAR_CORE_Probe_ProbeInfoUB: " + m_CI.debugName;
		ubCI.device = m_CI.device;
		ubCI.data = zero0;
		m_UB = CreateRef<Uniformbuffer<ProbeInfoUB>>(&ubCI);
	}
}

