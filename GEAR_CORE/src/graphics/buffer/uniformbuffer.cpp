#include "uniformbuffer.h"

using namespace GEAR;
using namespace GRAPHICS;

UniformBuffer::UniformBuffer(unsigned int size, unsigned int bindingIndex) //Only one UBO can be used with a single binding specifier. 
	:m_Size(size), m_BindingIndex(bindingIndex)
{
	glGenBuffers(1, &m_UniformID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UniformID);
	glBufferData(GL_UNIFORM_BUFFER, m_Size, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, m_BindingIndex, m_UniformID, 0, m_Size);
}

UniformBuffer::~UniformBuffer()
{
	glDeleteBuffers(1, &m_UniformID);
}

void UniformBuffer::Bind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_UniformID);
}

void UniformBuffer::Unbind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::SubDataBind(const float* data, unsigned int size, unsigned int offset) const
{
	if ((size + offset) <= m_Size)
	{
		Bind();
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		Unbind();
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::UniformBuffer: The size or offset is too large for the UniformBuffer." << std::endl;
	}
	
}
