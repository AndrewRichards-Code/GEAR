#include "object.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Object::Object(const char* objFilePath, Shader& shader, const Texture& texture, const Mat4& modl)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Texture(texture), m_ModlMatrix(modl)
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

	m_VAO = new VertexArray();
	VertexBuffer vbo0(&m_Vertices[0], m_Vertices.size(), 3);
	VertexBuffer vbo1(&m_TextCoords[0], m_TextCoords.size(), 2);
	VertexBuffer vbo2(&m_Normals[0], m_Normals.size(), 3);
	m_VAO->AddBuffer(&vbo0, GEAR_BUFFER_POSITIONS);
	m_VAO->AddBuffer(&vbo1, GEAR_BUFFER_TEXTCOORDS);
	m_VAO->AddBuffer(&vbo2, GEAR_BUFFER_NORMALS);
	
	m_IBO = new IndexBuffer(&m_Indices[0], m_Indices.size());
	
	SetUniformModlMatrix();
}
Object::Object(const char * objFilePath, Shader& shader, const Texture& texture, Vec3 position, Vec2 size)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Texture(texture), m_Position(position), m_Size(size)
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
Object::~Object()
{
	delete m_VAO;
	delete m_IBO;
}

void Object::BindTexture(int slot) const
{
	m_Shader.Enable();
	m_Texture.Bind(0);
	m_Shader.SetUniform<int>("u_Texture", { slot });
	m_Shader.Disable();
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
	SetUniformModlMatrix();
}