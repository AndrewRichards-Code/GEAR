#include "batchrenderer2d.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;
using namespace ARM;
using namespace CROSSPLATFORM;

BatchRenderer2D::BatchRenderer2D()
{
	Init();
}

BatchRenderer2D::~BatchRenderer2D()
{
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
	glEnableVertexAttribArray(GEAR_BUFFER_TEXTIDS);
	glEnableVertexAttribArray(GEAR_BUFFER_NORMALS);

	glVertexAttribPointer(GEAR_BUFFER_POSITIONS, 3, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*)0);
	glVertexAttribPointer(GEAR_BUFFER_TEXTCOORDS, 2, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(GEAR_BUFFER_TEXTIDS, 1, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*)(5 * sizeof(GLfloat)));
	glVertexAttribPointer(GEAR_BUFFER_NORMALS, 3, GL_FLOAT, GL_FALSE, GEAR_RENDERER_VERTEX_SIZE, (const GLvoid*)((6 * sizeof(GLfloat))));
	
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

	m_IBO = std::make_unique<IndexBuffer>(indicies, GEAR_RENDERER_INDICIES_SIZE);

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

void BatchRenderer2D::Submit(Object* obj)
{
	m_Object = obj;
	const char* file = obj->GetObjFileName();
	bool test = obj->forBatchRenderer2D;

	int textureSlot = 0;
	unsigned int textureID = obj->GetTextureID();

	if (test == true)
	{
		if (file == "res/obj/quad.obj")
		{
			bool found = false;
			for (int i = 0; i < (int)m_TextureSlots.size(); i++)
			{
				if (m_TextureSlots[i] == textureID)
				{
					textureSlot = i + 1;
					found = true;
					break;
				}
			}
			if (!found)
			{
				if (m_TextureSlots.size() >= GEAR_RENDERER_MAX_TEXTURE_SLOTS)
				{
					CloseMapBuffer();
					Flush();
					OpenMapBuffer();
				}
				m_TextureSlots.push_back(textureID);
				textureSlot = m_TextureSlots.size();
			}

			Object::VertexData temp[4];

			int j = 0, k = 0;
			for (int i = 0; i < 4; i++)
			{
				temp[i].m_Vertex.x = obj->GetVertices()[j + 0];
				temp[i].m_Vertex.y = obj->GetVertices()[j + 1];
				temp[i].m_Vertex.z = obj->GetVertices()[j + 2];
				temp[i].m_TextCoord.x = obj->GetTextCoords()[k + 0];
				temp[i].m_TextCoord.y = obj->GetTextCoords()[k + 1];

				temp[i].m_TextId = (float)textureSlot;
				//std::cout << "ObjectPtr V: S, I" << std::endl;
				//std::cout << obj << ", " << i << ": " << textureSlot << ", " << textureID << std::endl << std::endl;

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
			std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::BatchRender2D: Submitted object does not use 'res/obj/quad.obj' as its mesh data!" << std::endl << "BatchRender2D only supports quads!" << std::endl;
		}
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::BatchRender2D: Submitted object did not use the correct constructor!" << std::endl;
	}
}

void BatchRenderer2D::Flush()
{
	for (int i = 0; i < (int)m_TextureSlots.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_TextureSlots[i]);
	}
	
	Object::SetTextureArray(m_Object->GetShader());
	m_Object->SetUniformModlMatrix();
	m_Object->GetShader().Enable();
	glBindVertexArray(m_VAO);
	m_IBO->Bind();
	glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
	m_IBO->Unbind();
	glBindVertexArray(0);
	m_Object->GetShader().Disable();
	
	//DepthRender
	/*for (int i = 0; i < (signed int)m_Lights.size(); i++)
	{
		m_Lights[i]->GetDepthShader().Enable();
		m_Lights[i]->CalculateLightMVP();
		glBindVertexArray(m_VAO);
		m_IBO->Bind();
		glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
		m_IBO->Unbind();
		glBindVertexArray(0);
		m_Lights[i]->GetDepthShader().Disable();
	}*/

	m_IndexCount = 0;
}