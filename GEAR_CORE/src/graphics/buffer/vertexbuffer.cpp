#include "vertexbuffer.h"

using namespace GEAR;
using namespace GRAPHICS;

VertexBuffer::VertexBuffer(const float* data, unsigned int count, unsigned int componentcount)
	:m_Count(count), m_ComponentCount(componentcount)
{
	glGenBuffers(1, &m_VertexID);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexID);
	glBufferData(GL_ARRAY_BUFFER, m_Count * sizeof(float), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_VertexID);
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexID);
}

void VertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
