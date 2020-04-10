#include "vertexarray.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

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

void VertexArray::AddBuffer(std::shared_ptr<VertexBuffer> vbo, BufferType type)
{
	m_VertexBuffer.push_back(vbo);
	Bind();
	vbo->Bind();
	glVertexAttribPointer((GLuint)type, vbo->GetComponentCount(), GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray((GLuint)type);
	vbo->Unbind();
	Unbind();
}
