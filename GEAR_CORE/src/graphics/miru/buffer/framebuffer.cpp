#include "framebuffer.h"

using namespace GEAR;
using namespace GRAPHICS;

using namespace miru;
using namespace miru::crossplatform;

FrameBuffer::FrameBuffer(void* device, int width, int height, int multisample, bool cubeMap)
	:m_Device(device), m_Width(width), m_Height(height), m_Multisample(1), m_CubeMap(cubeMap)
{
	if (multisample % 2 != 0)
		m_Multisample = 1;
	else if (multisample > 8)
		m_Multisample = 8;
	else
		m_Multisample = multisample;

	m_FramebufferCI.debugName = "GEAR_CORE_FrameBuffer";
	m_FramebufferCI.device = m_Device;
	m_FramebufferCI.width = m_Width;
	m_FramebufferCI.height = m_Height;
	m_FramebufferCI.layers = 1;
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::UpdateFrameBufferSize(int width, int height)
{
	if (width != m_Width || height != m_Height)
	{
		m_Width = width;
		m_Height = height;
		for (int i = 0; i < static_cast<signed int>(m_ColourTextures.size()); i++)
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

void FrameBuffer::AddColourTextureAttachment(int attachment)
{
	m_FramebufferCI.attachments.resize(std::max((size_t)attachment + 1, m_FramebufferCI.attachments.size()));
	CheckColourTextureAttachments(attachment);
	m_ColourTextures[attachment] = std::make_shared<Texture>(m_Device, nullptr, 
		m_CubeMap ? Image::Type::TYPE_CUBE : Image::Type::TYPE_2D, Image::Format::R8G8B8A8_UNORM,
		m_Multisample, m_Width, m_Height, 1);
	m_FramebufferCI.attachments[attachment] = m_ColourTextures[attachment]->GetTextureImageView();
}

void FrameBuffer::AddDepthTextureAttachment(int attachment)
{
	m_FramebufferCI.attachments.resize(std::max((size_t)attachment + 1, m_FramebufferCI.attachments.size()));
	CheckColourTextureAttachments(attachment);
	m_ColourTextures[attachment] = m_DepthTexture = std::make_shared<Texture>(m_Device, nullptr,
		m_CubeMap ? Image::Type::TYPE_CUBE : Image::Type::TYPE_2D, Image::Format::D32_SFLOAT,
		m_Multisample, m_Width, m_Height, 1);
	if (m_DepthTexture->IsDepthTexture())
	{
		m_FramebufferCI.attachments[attachment] = m_DepthTexture->GetTextureImageView();
	}
}

void FrameBuffer::FinaliseFramebuffer(miru::Ref<miru::crossplatform::RenderPass> renderPass)
{
	m_FramebufferCI.renderPass = renderPass;
	m_Framebuffer = Framebuffer::Create(&m_FramebufferCI);
}

void FrameBuffer::CheckColourTextureAttachments(int attachment)
{
	if (attachment > static_cast<signed int>(m_ColourTextures.size()))
	{
		std::cout << "ERROR: GEAR::GRAPHICS::FrameBuffer: Attachment slot unavailable! Only 8 available slots." << std::endl;
		return;
	}

	if (m_ColourTextures[attachment] != nullptr)
	{
		std::cout << "ERROR: GEAR::GRAPHICS::FrameBuffer: Attachment slot is already taken!" << std::endl;
		return;
	}
}