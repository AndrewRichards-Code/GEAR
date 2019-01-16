#include "uniformbuffer.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

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
		/*if (m_BindingIndex == 2)
			PrintUBOData();*/
		Unbind();
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::UniformBuffer: The size or offset is too large for the UniformBuffer." << std::endl;
	}
	
}

void UniformBuffer::PrintUBOData() const
{
	Bind();
	system("CLS");
	std::vector<float> dataOut(m_Size);
	glGetBufferSubData(GL_UNIFORM_BUFFER, 0, m_Size, dataOut.data());

	for (unsigned int i = 0; i < (m_Size / 4); i += 4)
	{
		std::cout << "Offest: "<< 4 * i << " : "
		<< dataOut[i + 0] << ", "
		<< dataOut[i + 1] << ", "
		<< dataOut[i + 2] << ", "
		<< dataOut[i + 3] << ", "
		<< std::endl;
	}
	std::cout << std::endl;
	Unbind();
}

const float* const UniformBuffer::GetUBOData() const
{
	Bind();
	std::vector<float> dataOut(m_Size);
	glGetBufferSubData(GL_UNIFORM_BUFFER, 0, m_Size, dataOut.data());
	Unbind();
	return dataOut.data();

}