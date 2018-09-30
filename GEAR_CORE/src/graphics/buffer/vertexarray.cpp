#include "vertexarray.h"

using namespace GEAR;
using namespace GRAPHICS;

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_VertexArrayID);
	glBindVertexArray(m_VertexArrayID);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_VertexArrayID);
}

void VertexArray::Bind() const
{
	glBindVertexArray(m_VertexArrayID);
}

void VertexArray::Unbind() const
{
	glBindVertexArray(0);
}

void VertexArray::AddBuffer(std::shared_ptr<VertexBuffer> vbo, GLuint index)
{
	m_VertexBuffer.push_back(vbo);
	Bind();
	vbo->Bind();
	glVertexAttribPointer(index, vbo->GetComponentCount(), GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(index);
	vbo->Unbind();
	Unbind();
}
