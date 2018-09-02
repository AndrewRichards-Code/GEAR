#include "batchrenderer2d.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

BatchRenderer2D::BatchRenderer2D()
{

}

BatchRenderer2D::~BatchRenderer2D()
{
	delete m_IBO;
	glDeleteBuffers(1, &m_VBO);
}

void BatchRenderer2D::Init()
{
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, GEAR_RENDERER_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glEnableVertexAttribArray(GEAR_BUFFER_POSITIONS);
	glEnableVertexAttribArray(GEAR_BUFFER_TEXTCOORDS);
	glEnableVertexAttribArray(GEAR_BUFFER_NORMALS);
	glVertexAttribPointer(GEAR_BUFFER_POSITIONS, 3, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*) 0);
	glVertexAttribPointer(GEAR_BUFFER_TEXTCOORDS, 3, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(GEAR_BUFFER_NORMALS, 3, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*)(5 * sizeof(GLfloat)));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}