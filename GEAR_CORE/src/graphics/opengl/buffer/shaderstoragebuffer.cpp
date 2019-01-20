#include "shaderstoragebuffer.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

ShaderStorageBuffer::ShaderStorageBuffer(unsigned int size, unsigned int bindingIndex) //Only one SSBO can be used with a single binding specifier.
	:m_Size(size), m_BindingIndex(bindingIndex)
{
	glGenBuffers(1, &m_ShaderStorageID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ShaderStorageID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_Size, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, m_BindingIndex, m_ShaderStorageID, 0, m_Size);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
	glDeleteBuffers(1, &m_ShaderStorageID);
}

void ShaderStorageBuffer::Bind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ShaderStorageID);
}

void ShaderStorageBuffer::Unbind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ShaderStorageBuffer::Access(float* data, unsigned int size, unsigned int offset, ShaderStorageAccess access) const
{
	if ((size + offset) <= m_Size)
	{
		Bind();
		void* gpuData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, (GLbitfield)access);
		
		if ((access & GEAR_MAP_READ_BIT) != 0)
			memcpy(data, gpuData, size);
		if ((access & GEAR_MAP_WRITE_BIT) != 0)
			memcpy(gpuData, data, size);
		if((access & GEAR_MAP_READ_BIT) != 0 && (access & GEAR_MAP_WRITE_BIT) != 0)
			data = static_cast<float*>(gpuData);

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		Unbind();
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::ShaderStorageBuffer: The size or offset is too large for the ShaderStorageBuffer." << std::endl;
	}
}

void ShaderStorageBuffer::PrintUBOData() const
{
	Bind();
	system("CLS");
	std::vector<float> dataOut(m_Size);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_Size, dataOut.data());

	for (unsigned int i = 0; i < (m_Size / 4); i += 4)
	{
		std::cout << "Offest: " << 4 * i << " : "
			<< dataOut[i + 0] << ", "
			<< dataOut[i + 1] << ", "
			<< dataOut[i + 2] << ", "
			<< dataOut[i + 3] << ", "
			<< std::endl;
	}
	std::cout << std::endl;
	Unbind();
}

const float* const ShaderStorageBuffer::GetUBOData() const
{
	Bind();
	std::vector<float> dataOut(m_Size);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_Size, dataOut.data());
	Unbind();
	return dataOut.data();
}
