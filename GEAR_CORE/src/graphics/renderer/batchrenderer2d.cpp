#include "batchrenderer2d.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

BatchRenderer2D::BatchRenderer2D()
{
	Init();
}

BatchRenderer2D::~BatchRenderer2D()
{
	delete m_IBO;
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
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

	glVertexAttribPointer(GEAR_BUFFER_POSITIONS, 3, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*)0);
	glVertexAttribPointer(GEAR_BUFFER_TEXTCOORDS, 3, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(GEAR_BUFFER_NORMALS, 3, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*)(5 * sizeof(GLfloat)));
	
	GLuint indicies[GEAR_RENDERER_INDICIES_SIZE];
	int offset = 0;
	for (int i = 0; i < GEAR_RENDERER_INDICIES_SIZE; i += 6)
	{
		indicies[i + 0] = offset + 0;
		indicies[i + 1] = offset + 1;
		indicies[i + 2] = offset + 2;
		indicies[i + 3] = offset + 2;
		indicies[i + 4] = offset + 3;
		indicies[i + 5] = offset + 0;
		offset += 4;
	}

	m_IBO = new IndexBuffer(indicies, GEAR_RENDERER_INDICIES_SIZE);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void BatchRenderer2D::OpenMapBuffer() 
{
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	m_BatchBuffer = (Object::VertexData*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void BatchRenderer2D::CloseMapBuffer() 
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BatchRenderer2D::Submit(const Object* obj)
{
	m_Object = obj;
	const char* file = obj->GetObjFileName();
	bool test = obj->forBatchRenderer2D;

	if (test = true)
	{
		if (file = "res/obj/quad.obj")
		{
			Object::VertexData temp[4];

			int j = 0, k = 0;
			for (int i = 0; i < 4; i++)
			{
				temp[i].m_Vertex.x = obj->GetVertices()[j + 0];
				temp[i].m_Vertex.y = obj->GetVertices()[j + 1];
				temp[i].m_Vertex.z = obj->GetVertices()[j + 2];
				temp[i].m_TextCoord.x = obj->GetTextCoords()[k + 0];
				temp[i].m_TextCoord.y = obj->GetTextCoords()[k + 1];
				temp[i].m_Normal.x = obj->GetNormals()[j + 0];
				temp[i].m_Normal.y = obj->GetNormals()[j + 1];
				temp[i].m_Normal.z = obj->GetNormals()[j + 2];
				j += 3;
				k += 2;

				*m_BatchBuffer = temp[i];
				m_BatchBuffer++;
			}

			m_IndexCount += 6;
		}
		else
		{
			std::cout << "ERROR: GEAR::GRAPHICS::BatchRender2D: Submitted object does not use 'res/obj/quad.obj' has its mesh data!" << std::endl << "BatchRender2D only supports quad!" << std::endl;

		}
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::BatchRender2D: Submitted object did not use the correct constructor!" << std::endl;

	}
}

void BatchRenderer2D::Flush()
{
	m_Object->BindTexture(0);

	m_Object->GetShader().Enable();
	glBindVertexArray(m_VAO);
	m_IBO->Bind();
	glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
	m_IBO->Unbind();
	glBindVertexArray(0);
	m_Object->GetShader().Disable();

	m_IndexCount = 0;
}