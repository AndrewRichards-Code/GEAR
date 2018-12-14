#include "framebuffer.h"

using namespace GEAR;
using namespace GRAPHICS;

FrameBuffer::FrameBuffer(const Window& window)
	:m_Window(window)
{
	glGenFramebuffers(1, &m_FrameID);
	glGenRenderbuffers(1, &m_RenderBufferID);
	Bind();

	AddColourTextureAttachment(0);
	AddDepthTextureAttachment();
	AddDepthBufferAttachment();

	if (GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR: GEAR::GRAPHICS::FrameBuffer: FrameBuffer is not complete! Code: " << error << std::endl;
		throw(0);
	}

	Unbind();
}

FrameBuffer::~FrameBuffer()
{
	glDeleteRenderbuffers(1, &m_RenderBufferID);
	glDeleteFramebuffers(1, &m_FrameID);
}

void FrameBuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameID);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBufferID);
}

void FrameBuffer::Unbind() const
{
	glBindRenderbuffer(GL_RENDERBUFFER,0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::UpdateFrameBufferSize()
{
	if (m_Window.GetWidth() != m_DepthTexture->GetWidth() || m_Window.GetHeight() != m_DepthTexture->GetHeight())
	{
		for (int i = 0; i < static_cast<signed int>(m_ColourTextures.size()); i++)
		{
			if (m_ColourTextures[i] != nullptr)
			{
				m_ColourTextures[i] = nullptr;
				AddColourTextureAttachment(i);
			}
		}
		AddDepthTextureAttachment();
		AddDepthBufferAttachment();
	}
}

void FrameBuffer::AddColourTextureAttachment(int attachment)
{
	if (attachment > static_cast<signed int>(m_ColourTextures.size()))
	{
		std::cout << "ERROR: GEAR::GRAPHICS::FrameBuffer: Attachment slot unavailable! Only 16 available slots." << std::endl;
		return;
	}
	
	if (m_ColourTextures[attachment] != nullptr)
	{
		std::cout << "ERROR: GEAR::GRAPHICS::FrameBuffer: Attachment slot is already taken!" << std::endl;
		return;
	}

	m_ColourTextures[attachment] = std::make_shared<Texture>(m_Window.GetWidth(), m_Window.GetHeight(), false);
	if (m_ColourTextures[attachment]->IsDepthTexture() == false)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, m_ColourTextures[attachment]->GetTextureID(), 0);
	}
}

void FrameBuffer::UseColourTextureAttachment(int attachment)
{
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);
}

void FrameBuffer::AddDepthTextureAttachment()
{
	m_DepthTexture = std::make_shared<Texture>(m_Window.GetWidth(), m_Window.GetHeight(), true);
	if (m_DepthTexture->IsDepthTexture() == true)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture->GetTextureID(), 0);
	}
}

void FrameBuffer::AddDepthBufferAttachment()
{
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Window.GetWidth(), m_Window.GetHeight()); 
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RenderBufferID);
}