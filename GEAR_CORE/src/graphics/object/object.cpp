#include "object.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Object::Object(const char* objFilePath, Shader& shader, const Texture& texture, const Mat4& modl)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Texture(texture), m_ModlMatrix(modl)
{
	LoadObjDataIntoObject();
	GenVAOandIBO();	
	BindTexture(0);
	SetUniformModlMatrix();
}

Object::Object(const char* objFilePath, Shader& shader, const Texture& texture, const Vec3& position, const Vec2& size)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Texture(texture), m_Position(position), m_Size(size)
{
	m_ObjData = FileUtils::read_obj(m_ObjFilePath);

	m_Vertices.reserve(m_ObjData.GetSizeVertices() * 3);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec3 temp = m_ObjData.m_Vertices[(unsigned int)m_ObjData.m_UniqueVertices[i].x];
		temp.x *= size.x;
		temp.y *= size.y;
		temp = temp + m_Position;
		m_Vertices.push_back(temp.x);
		m_Vertices.push_back(temp.y);
		m_Vertices.push_back(temp.z);
	}

	m_TextCoords.reserve(m_ObjData.GetSizeVertices() * 2);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec2 temp = m_ObjData.m_TexCoords[(unsigned int)m_ObjData.m_UniqueVertices[i].y];
		m_TextCoords.push_back(temp.x);
		m_TextCoords.push_back(temp.y);;
	}

	m_Normals.reserve(m_ObjData.GetSizeVertices() * 3);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec3 temp = m_ObjData.m_Normals[(unsigned int)m_ObjData.m_UniqueVertices[i].z];
		m_Normals.push_back(temp.x);
		m_Normals.push_back(temp.y);
		m_Normals.push_back(temp.z);
	}

	m_Indices.reserve(m_ObjData.GetSizeVertIndices());
	for (int i = 0; i < m_ObjData.GetSizeVertIndices(); i++)
	{
		m_Indices.push_back(m_ObjData.m_VertIndices[i]);
	}

	SetUniformModlMatrix(Mat4::Identity());
	forBatchRenderer2D = true;
}

Object::~Object()
{
	if (forBatchRenderer2D = false)
	{
		delete m_VAO;
		delete m_IBO;
	}
}

void Object::BindTexture(int slot) const
{
	m_Shader.Enable();
	m_Texture.Bind(slot);
	m_Shader.SetUniform<int>("u_Texture", { slot });
	m_Shader.Disable();
}

void Object::UnbindTexture() const
{
	m_Texture.Unbind();
}

void Object::SetTextureArray(Shader& shader)
{
	int m_TextIDs[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	shader.Enable();
	shader.SetUniformArray<int>("u_Textures", 1, 32, m_TextIDs);
	shader.Disable();
}

void Object::UnsetTextureArray(Shader& shader)
{
	shader.Enable();
	shader.SetUniformArray<int>("u_Textures", 1, 32, nullptr);
	shader.Disable();
}

void Object::SetUniformModlMatrix() const
{
	m_Shader.Enable();
	m_Shader.SetUniformMatrix<4>("u_Modl", 1, GL_TRUE, m_ModlMatrix.a);
	m_Shader.Disable();
}

void Object::SetUniformModlMatrix(const Mat4& modl)
{
	m_ModlMatrix = modl;
	m_Shader.Enable();
	m_Shader.SetUniformMatrix<4>("u_Modl", 1, GL_TRUE, m_ModlMatrix.a);
	m_Shader.Disable();
}

void Object::LoadObjDataIntoObject()
{
	m_ObjData = FileUtils::read_obj(m_ObjFilePath);

	m_Vertices.reserve(m_ObjData.GetSizeVertices() * 3);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec3 temp = m_ObjData.m_Vertices[(unsigned int)m_ObjData.m_UniqueVertices[i].x];
		m_Vertices.push_back(temp.x);
		m_Vertices.push_back(temp.y);
		m_Vertices.push_back(temp.z);
	}

	m_TextCoords.reserve(m_ObjData.GetSizeVertices() * 2);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec2 temp = m_ObjData.m_TexCoords[(unsigned int)m_ObjData.m_UniqueVertices[i].y];
		m_TextCoords.push_back(temp.x);
		m_TextCoords.push_back(temp.y);;
	}

	m_Normals.reserve(m_ObjData.GetSizeVertices() * 3);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec3 temp = m_ObjData.m_Normals[(unsigned int)m_ObjData.m_UniqueVertices[i].z];
		m_Normals.push_back(temp.x);
		m_Normals.push_back(temp.y);
		m_Normals.push_back(temp.z);
	}

	m_Indices.reserve(m_ObjData.GetSizeVertIndices());
	for (int i = 0; i < m_ObjData.GetSizeVertIndices(); i++)
	{
		m_Indices.push_back(m_ObjData.m_VertIndices[i]);
	}
}

void Object::GenVAOandIBO()
{
	if (m_Vertices.size() && m_TextCoords.size() && m_Normals.size() && m_Indices.size() > 0)
	{
		m_VAO = new VertexArray;
		m_VAO->AddBuffer(new VertexBuffer(&m_Vertices[0], m_Vertices.size(), 3), GEAR_BUFFER_POSITIONS);
		m_VAO->AddBuffer(new VertexBuffer(&m_TextCoords[0], m_TextCoords.size(), 2), GEAR_BUFFER_TEXTCOORDS);
		m_VAO->AddBuffer(new VertexBuffer(&m_Normals[0], m_Normals.size(), 3), GEAR_BUFFER_NORMALS);

		m_IBO = new IndexBuffer(&m_Indices[0], m_Indices.size());
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::Object: Unable to construct VAO, VBO, IBO! All data arrays have size = 0!" << std::endl;
	}
}