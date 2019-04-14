#include "framebuffer.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

FrameBuffer::FrameBuffer(int width, int height, int multisample, bool cubeMap, Texture::ImageFormat format)
	:m_Width(width), m_Height(height), m_Multisample(1), m_CubeMap(cubeMap), m_Format(format)
{
	if (multisample % 2 != 0)
		m_Multisample = 1;
	else if (multisample > 8)
		m_Multisample = 8;
	else
		m_Multisample = multisample;

	glGenFramebuffers(1, &m_FrameID);
	glGenRenderbuffers(1, &m_RenderBufferID);
	if (m_Multisample > 1)
		m_ResolvedFBO = std::make_unique<FrameBuffer>(m_Width, m_Height, 1, m_CubeMap, m_Format);
	
	Bind();

	AddColourTextureAttachment(0);
	AddDepthTextureAttachment();
	AddDepthBufferAttachment();

	if (GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::FrameBuffer: FrameBuffer is not complete! Code: " << error << std::endl;
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

void FrameBuffer::BindResolved() const
{
	if (m_ResolvedFBO)
		glBindFramebuffer(GL_FRAMEBUFFER, m_ResolvedFBO->m_FrameID);
	else
		Bind();
}

void FrameBuffer::UnbindResolved() const
{
	if (m_ResolvedFBO)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	else
		Unbind();
}

void FrameBuffer::UpdateFrameBufferSize(int width, int height)
{
	if (width != m_DepthTexture->GetWidth() || height != m_DepthTexture->GetHeight())
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
		AddDepthBufferAttachment();
	}
}

void FrameBuffer::AddColourTextureAttachment(int attachment)
{
	if (!m_CubeMap)
	{
		CheckColourTextureAttachments(attachment);
		m_ColourTextures[attachment] = std::make_shared<Texture>(m_Width, m_Height, false, m_Multisample, m_Format);
		if (m_ColourTextures[attachment]->IsDepthTexture() == false)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, m_Multisample > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, m_ColourTextures[attachment]->GetTextureID(), 0);
	}
	else
	{
		for (int i = 0; i < 6; i++)
		{
			CheckColourTextureAttachments(attachment + i);
			m_ColourTextures[attachment + i] = std::make_shared<Texture>(m_Width, m_Height, false, m_Multisample, m_Format);
			if (m_ColourTextures[attachment + i]->IsDepthTexture() == false)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment + i, m_Multisample > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, m_ColourTextures[attachment + i]->GetTextureID(), 0);
		}
	}
}

void FrameBuffer::DrawToColourTextureAttachment(int attachment)
{
	if(m_ColourTextures[attachment])
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);
	else
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::FrameBuffer: Can't draw to attachment "<< attachment << ". No associated texture." << std::endl;
}
void FrameBuffer::ReadFromColourTextureAttachment(int attachment)
{
	if (m_ColourTextures[attachment])
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);
	else
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::FrameBuffer: Can't read from attachment " << attachment << ". No associated texture." << std::endl;
}

void FrameBuffer::Resolve()
{
	if (m_ResolvedFBO == nullptr)
		return;

	//Adds any addition attachments to the resolvedFBO.
	int start = m_CubeMap ? 6 : 1; //The Constructor provides a colour attachment at 0, 0-5 for a cubemap.
	m_ResolvedFBO->Bind();
	for (int i = start; i < static_cast<signed int>(m_ResolvedFBO->m_ColourTextures.size()); i++)
	{
		if (m_ColourTextures[i] == nullptr)
			break;
		else
			m_ResolvedFBO->AddColourTextureAttachment(i);
	}
	m_ResolvedFBO->Unbind();


	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ResolvedFBO->m_FrameID);
	for (int i = 0; i < static_cast<signed int>(m_ResolvedFBO->m_ColourTextures.size()); i++)
	{
		if (m_ColourTextures[i] == nullptr)
			break;

		ReadFromColourTextureAttachment(i);
		DrawToColourTextureAttachment(i);
		glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, m_Width, m_Height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::AddDepthTextureAttachment()
{
	m_DepthTexture = std::make_shared<Texture>(m_Width, m_Height, true, m_Multisample, m_Format);
	if (m_DepthTexture->IsDepthTexture() == true)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture->GetTextureID(), 0);
	}
}

void FrameBuffer::AddDepthBufferAttachment()
{
	if (m_Multisample > 1)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_Multisample, GL_DEPTH_COMPONENT, m_Width, m_Height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height); 
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RenderBufferID);
}

void FrameBuffer::CheckColourTextureAttachments(int attachment)
{
	if (attachment > static_cast<signed int>(m_ColourTextures.size()))
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::FrameBuffer: Attachment slot unavailable! Only 16 available slots." << std::endl;
		return;
	}

	if (m_ColourTextures[attachment] != nullptr)
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::FrameBuffer: Attachment slot is already taken!" << std::endl;
		return;
	}
}