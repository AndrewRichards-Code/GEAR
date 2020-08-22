#include "gear_core_common.h"
#include "Framebuffer.h"

using namespace gear;
using namespace graphics;

Framebuffer::Framebuffer(CreateInfo* pCreateInfo)
{
	m_FramebufferCI.debugName = "GEAR_CORE_FrameBuffer: " + m_CI.debugName;
	m_FramebufferCI.device = m_CI.device;
	m_FramebufferCI.width = m_CI.width;
	m_FramebufferCI.height = m_CI.height;
	m_FramebufferCI.layers = 1;
	m_FramebufferCI.renderPass = m_CI.renderPass;
}

Framebuffer::~Framebuffer()
{
}

void Framebuffer::UpdateFrameBufferSize(uint32_t width, uint32_t height)
{
	if (width != m_CI.width || height != m_CI.height)
	{
		m_CI.width = width;
		m_CI.height = height;
		for (size_t i = 0; i < m_ColourTextures.size(); i++)
		{
			if (m_ColourTextures[i] != nullptr)
			{
				m_ColourTextures[i] = nullptr;
				AddColourTextureAttachment(i);
			}
		}
		AddDepthTextureAttachment();
	}
}

void Framebuffer::AddColourTextureAttachment(size_t attachment)
{
	m_FramebufferCI.attachments.resize(std::max(attachment + 1, m_FramebufferCI.attachments.size()));
	CheckColourTextureAttachments(attachment);
	
	Texture::CreateInfo colourTextureCI;
	colourTextureCI.device = m_CI.device;
	colourTextureCI.filepaths = {};
	colourTextureCI.data = nullptr;
	colourTextureCI.size = 0;
	colourTextureCI.width = m_CI.width;
	colourTextureCI.height = m_CI.height;
	colourTextureCI.depth = 1;
	colourTextureCI.format = miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
	colourTextureCI.type = m_CI.cubemap ? miru::crossplatform::Image::Type::TYPE_CUBE : miru::crossplatform::Image::Type::TYPE_2D;
	colourTextureCI.samples = m_CI.samples;
	m_ColourTextures[attachment] = gear::CreateRef<Texture>(&colourTextureCI);
	
	m_FramebufferCI.attachments[attachment] = m_ColourTextures[attachment]->GetTextureImageView();
}

void Framebuffer::AddDepthTextureAttachment(size_t attachment)
{
	m_FramebufferCI.attachments.resize(std::max(attachment + 1, m_FramebufferCI.attachments.size()));
	CheckColourTextureAttachments(attachment);
	
	Texture::CreateInfo depthTextureCI;
	depthTextureCI.device = m_CI.device;
	depthTextureCI.filepaths = {};
	depthTextureCI.data = nullptr;
	depthTextureCI.size = 0;
	depthTextureCI.width = m_CI.width;
	depthTextureCI.height = m_CI.height;
	depthTextureCI.depth = 1;
	depthTextureCI.format = miru::crossplatform::Image::Format::D32_SFLOAT;
	depthTextureCI.type = m_CI.cubemap ? miru::crossplatform::Image::Type::TYPE_CUBE : miru::crossplatform::Image::Type::TYPE_2D;
	depthTextureCI.samples = m_CI.samples;
	m_ColourTextures[attachment] = m_DepthTexture = std::make_shared<Texture>(&depthTextureCI);
	
	if (m_DepthTexture->IsDepthTexture())
	{
		m_FramebufferCI.attachments[attachment] = m_DepthTexture->GetTextureImageView();
	}
}

void Framebuffer::FinaliseFramebuffer()
{
	m_Framebuffer = miru::crossplatform::Framebuffer::Create(&m_FramebufferCI);
}

void Framebuffer::CheckColourTextureAttachments(size_t attachment)
{
	if (attachment > m_ColourTextures.size())
	{
		GEAR_LOG(core::Log::Level::ERROR, core::Log::ErrorCode::GRAPHICS | core::Log::ErrorCode::INVALID_VALUE, "Attachment slot unavailable! Only 8 available slots.");
		return;
	}

	if (m_ColourTextures[attachment] != nullptr)
	{
		GEAR_LOG(core::Log::Level::ERROR, core::Log::ErrorCode::GRAPHICS | core::Log::ErrorCode::INVALID_VALUE, "Attachment slot is already taken.");
		return;
	}
}