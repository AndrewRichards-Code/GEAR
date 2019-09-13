#include "batchRenderer3d.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;
using namespace ARM;
using namespace CROSSPLATFORM;

BatchRenderer3D::BatchRenderer3D()
{
	glGenBuffers(1, &m_VBO);
	glGenVertexArrays(1, &m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, GEAR_BATCH_RENDERER_3D_BUFFER_SIZE, nullptr, GL_DYNAMIC_DRAW);

	glBindVertexArray(m_VAO);

	glEnableVertexAttribArray((GLuint)VertexArray::BufferType::GEAR_BUFFER_POSITIONS);
	glEnableVertexAttribArray((GLuint)VertexArray::BufferType::GEAR_BUFFER_TEXCOORDS);
	glEnableVertexAttribArray((GLuint)VertexArray::BufferType::GEAR_BUFFER_TEXIDS);
	glEnableVertexAttribArray((GLuint)VertexArray::BufferType::GEAR_BUFFER_NORMALS);
	glEnableVertexAttribArray((GLuint)VertexArray::BufferType::GEAR_BUFFER_COLOURS);

	glVertexAttribPointer((GLuint)VertexArray::BufferType::GEAR_BUFFER_POSITIONS, 3, GL_FLOAT, GL_FALSE, GEAR_BATCH_RENDERER_3D_VERTEX_SIZE, (const GLvoid*)offsetof(Object::VertexData, m_Vertex));
	glVertexAttribPointer((GLuint)VertexArray::BufferType::GEAR_BUFFER_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, GEAR_BATCH_RENDERER_3D_VERTEX_SIZE, (const GLvoid*)offsetof(Object::VertexData, m_TexCoord));
	glVertexAttribPointer((GLuint)VertexArray::BufferType::GEAR_BUFFER_TEXIDS, 1, GL_FLOAT, GL_FALSE, GEAR_BATCH_RENDERER_3D_VERTEX_SIZE, (const GLvoid*)offsetof(Object::VertexData, m_TexId));
	glVertexAttribPointer((GLuint)VertexArray::BufferType::GEAR_BUFFER_NORMALS, 3, GL_FLOAT, GL_FALSE, GEAR_BATCH_RENDERER_3D_VERTEX_SIZE, (const GLvoid*)offsetof(Object::VertexData, m_Normal));
	glVertexAttribPointer((GLuint)VertexArray::BufferType::GEAR_BUFFER_COLOURS, 4, GL_FLOAT, GL_FALSE, GEAR_BATCH_RENDERER_3D_VERTEX_SIZE, (const GLvoid*)offsetof(Object::VertexData, m_Colour));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

BatchRenderer3D::~BatchRenderer3D()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void BatchRenderer3D::OpenMapBuffer() 
{
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	m_BatchBuffer = (Object::VertexData*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void BatchRenderer3D::CloseMapBuffer() 
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BatchRenderer3D::Submit(Object* obj)
{
	m_Object = obj;

	int textureSlot = 0;
	unsigned int textureID = obj->GetTextureID();

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
		m_TextureSlots.push_back(textureID);
		if (m_TextureSlots.size() >= GEAR_BATCH_RENDERER_3D_MAX_TEXTURE_SLOTS)
		{
			CloseMapBuffer();
			Flush();
			OpenMapBuffer();
		}
		textureSlot = static_cast<int>(m_TextureSlots.size());
	}

	Object::VertexData temp;

	int j = 0, k = 0, l = 0;
	for (int i = 0; i < obj->GetNumOfUniqueVertices(); i++)
	{
		Vec4 ModlVertex = obj->GetModlMatrix() * Vec4(obj->GetVertices()[j + 0], obj->GetVertices()[j + 1], obj->GetVertices()[j + 2], 1);
		temp.m_Vertex = Vec3(ModlVertex.x, ModlVertex.y, ModlVertex.z);

		temp.m_TexCoord.x = obj->GetTextCoords()[k + 0];
		temp.m_TexCoord.y = obj->GetTextCoords()[k + 1];

		temp.m_TexId = static_cast<float>(textureSlot);

		temp.m_Normal.x = obj->GetNormals()[j + 0];
		temp.m_Normal.y = obj->GetNormals()[j + 1];
		temp.m_Normal.z = obj->GetNormals()[j + 2];

		if (!obj->GetColours().empty())
		{
			temp.m_Colour.r = obj->GetColours()[l + 0];
			temp.m_Colour.g = obj->GetColours()[l + 1];
			temp.m_Colour.b = obj->GetColours()[l + 2];
			temp.m_Colour.a = obj->GetColours()[l + 3];
		}
		else
		{
			temp.m_Colour = Vec4(0, 0, 0, 1);
		}

		j += 3;
		k += 2;
		l += 4;

		*m_BatchBuffer = temp;
		m_BatchBuffer++;
	}

	//Add to the Array of the Counts
	m_Counts.push_back(obj->GetIBO()->GetCount());

	//Offset Indicies by PreviousNumOfVertices
	for (auto& index : obj->GetIndices())
		m_IBOIndicies.emplace_back(index + m_PreviousNumOfAccumulatedVertices);

	m_PreviousNumOfAccumulatedVertices += obj->GetNumOfUniqueVertices();
}

void BatchRenderer3D::Flush()
{
	//Construct IBO
	if(m_IBO == nullptr)
		m_IBO = std::make_unique<IndexBuffer>(m_IBOIndicies.data(), m_IBOIndicies.size());

	//Find offsets into the index buffer and copy to OpenGL format
	for (size_t i = 0; i < m_Counts.size(); i++)
	{
		if (i == 0)
			m_Indicies.push_back(0);
		else
			m_Indicies.push_back((m_Counts[i - 1] * sizeof(unsigned int)) + m_Indicies[i - 1]);
	}
	for (auto& index : m_Indicies)
		m_GLIndicies.push_back((GLvoid*)index);
	
	//Bind textures
	for (int i = 0; i < (int)m_TextureSlots.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_TextureSlots[i]);

		if (i > GEAR_BATCH_RENDERER_3D_MAX_TEXTURE_SLOTS - 1)
		{
			m_TextureSlots.erase(m_TextureSlots.begin(), m_TextureSlots.begin() + GEAR_BATCH_RENDERER_3D_MAX_TEXTURE_SLOTS - 1);
			break;
		}
	}

	//Draw Call
	Object::SetTextureArray(m_Object->GetShader());
	m_Object->SetUniformModlMatrix(Mat4::Identity());
	m_Object->GetShader().Enable();
	glBindVertexArray(m_VAO);
	m_IBO->Bind();
	glMultiDrawElements(GL_TRIANGLES, m_Counts.data(), GL_UNSIGNED_INT, (const void* const *)m_GLIndicies.data(), (GLsizei)m_Counts.size());
	m_IBO->Unbind();
	glBindVertexArray(0);
	m_Object->GetShader().Disable();

	//Clearing
	m_Counts.clear();
	m_Indicies.clear();
	m_GLIndicies.clear();
	m_IBOIndicies.clear();
	m_PreviousNumOfAccumulatedVertices = 0;
}

void GEAR::GRAPHICS::OPENGL::BatchRenderer3D::Flush(const OPENGL::Shader * overrideShader)
{
	//Construct IBO
	if (m_IBO == nullptr)
		m_IBO = std::make_unique<IndexBuffer>(m_IBOIndicies.data(), m_IBOIndicies.size());

	//Find offsets into the index buffer and copy to OpenGL format
	for (size_t i = 0; i < m_Counts.size(); i++)
	{
		if (i == 0)
			m_Indicies.push_back(0);
		else
			m_Indicies.push_back((m_Counts[i - 1] * sizeof(unsigned int)) + m_Indicies[i - 1]);
	}
	for (auto& index : m_Indicies)
		m_GLIndicies.push_back((GLvoid*)(index));

	//Bind textures
	for (int i = 0; i < (int)m_TextureSlots.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_TextureSlots[i]);

		if (i > GEAR_BATCH_RENDERER_3D_MAX_TEXTURE_SLOTS - 1)
		{
			m_TextureSlots.erase(m_TextureSlots.begin(), m_TextureSlots.begin() + GEAR_BATCH_RENDERER_3D_MAX_TEXTURE_SLOTS - 1);
			break;
		}
	}

	//Draw Call
	Object::SetTextureArray(*overrideShader);
	m_Object->SetUniformModlMatrix(Mat4::Identity());
	overrideShader->Enable();
	glBindVertexArray(m_VAO);
	m_IBO->Bind();
	glMultiDrawElements(GL_TRIANGLES, m_Counts.data(), GL_UNSIGNED_INT, (const void* const *)m_GLIndicies.data(), (GLsizei)m_Counts.size());
	m_IBO->Unbind();
	glBindVertexArray(0);
	overrideShader->Disable();

	//Clearing
	m_Counts.clear();
	m_Indicies.clear();
	m_GLIndicies.clear();
	m_IBOIndicies.clear();
	m_PreviousNumOfAccumulatedVertices = 0;
}
