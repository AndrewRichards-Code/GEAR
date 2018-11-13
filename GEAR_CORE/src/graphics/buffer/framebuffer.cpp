#include "framebuffer.h"

using namespace GEAR;
using namespace GRAPHICS;

FrameBuffer::FrameBuffer(const Window& window, const Shader& shader)
	:m_Window(window), m_Shader(shader)
{
	glGenFramebuffers(1, &m_FrameID);
	glGenRenderbuffers(1, &m_RenderBufferID);
	Bind();
	AddDepthBuffer();
	AttachDepthTexture();

	if (GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR: GEAR::GRAPHICS::FrameBuffer: FrameBuffer is not complete! Code: " << error << std::endl;
		throw(0);
	}
	m_Quad = std::make_unique<Object>("res/obj/quad.obj", m_Shader, *m_DepthTexture, ARM::Mat4::Identity());

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
		AttachDepthTexture();
		AddDepthBuffer();
		m_Quad = std::make_unique<Object>("res/obj/quad.obj", m_Shader, *m_DepthTexture, ARM::Mat4::Identity());
	}
}

Object FrameBuffer::UseFrameBufferAsObject(const ARM::Vec3& translate, const ARM::Vec3& scale)
{
	m_Shader.Enable();
	m_Shader.SetUniformMatrix<4>("u_Proj", 1, GL_TRUE, ARM::Mat4::Identity().a);
	m_Shader.SetUniformMatrix<4>("u_View", 1, GL_TRUE, (ARM::Mat4::Translation(translate) * ARM::Mat4::Scale(scale)).a);
	m_Shader.Disable();
	return *m_Quad;
}

//private:
void FrameBuffer::AttachDepthTexture()
{
	m_DepthTexture = std::make_unique<Texture>(m_Window.GetWidth(), m_Window.GetHeight());
	if (m_DepthTexture->IsDepthTexture() == true)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_DepthTexture->GetTextureID(), 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture->GetTextureID(), 0);
	}
}

void FrameBuffer::AddDepthBuffer()
{
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Window.GetWidth(), m_Window.GetHeight()); 
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RenderBufferID);
}