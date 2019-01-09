#include "object.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace CROSSPLATFORM;
using namespace ARM;

Object::Object(const char* objFilePath, const OPENGL::Shader& shader, const OPENGL::Texture& texture, const Mat4& modl)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Texture(texture), m_ModlMatrix(modl)
{
	LoadObjDataIntoObject();
	GenVAOandIBO();
	if (m_Texture.IsCubeMap() == true)
	{
		BindCubeMap(0);
	}
	else
	{
		BindTexture(0);
	}
	SetUniformModlMatrix();
}

Object::Object(const char* objFilePath, const OPENGL::Shader& shader, const ARM::Vec4& colour, const ARM::Mat4& modl)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Colour(colour), m_ModlMatrix(modl)
{
	LoadObjDataIntoObject();
	m_Colours.resize(2 * m_TextCoords.size());
	for (int i = 0; i < (int)m_Colours.size();)
	{
		m_Colours[i + 0] = m_Colour.r;
		m_Colours[i + 1] = m_Colour.g;
		m_Colours[i + 2] = m_Colour.b;
		m_Colours[i + 3] = m_Colour.a;
		i += 4;
	}
	GenVAOandIBO();
	m_VAO->AddBuffer(std::make_shared<OPENGL::VertexBuffer>(&m_Colours[0], m_Colours.size(), 4), GEAR_BUFFER_COLOUR);
	SetUniformModlMatrix();
}

Object::Object(const char* objFilePath, const OPENGL::Shader& shader, const OPENGL::Texture& texture, const Vec3& position, const Vec2& size)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Texture(texture), m_Position(position), m_Size(size)
{
	m_ObjData.m_Vertices = { Vec3(-1, -1, 0), Vec3(1, -1, 0), Vec3(1, 1, 0), Vec3(-1, 1, 0) };
	m_ObjData.m_TexCoords = { Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1) };
	m_ObjData.m_Normals = { Vec3(0, 0, 1) };
	m_ObjData.m_VertIndices = { 0, 1, 2, 2, 3, 0 };
	m_ObjData.m_TextIndices = { 0, 1, 2, 2, 3, 0 };
	m_ObjData.m_NormIndices = { 0, 1, 2, 2, 3, 0 };
	m_ObjData.m_UniqueVertices = { Vec3(0, 0, 0), Vec3(1, 1, 0), Vec3(2, 2, 0), Vec3(3, 3, 0) };

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
		m_TextCoords.push_back(temp.x * m_Texture.GetTileFactor());
		m_TextCoords.push_back(temp.y * m_Texture.GetTileFactor());;
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

void Object::SetTextureArray(const OPENGL::Shader& shader)
{
	int m_TextIDs[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
	shader.Enable();
	shader.SetUniformArray<int>("u_Textures", 1, 32, m_TextIDs);
	shader.Disable();
}

void Object::UnsetTextureArray(const OPENGL::Shader& shader)
{
	shader.Enable();
	shader.SetUniformArray<int>("u_Textures", 1, 32, nullptr);
	shader.Disable();
}

void Object::BindCubeMap(int slot) const
{
	m_Shader.Enable();
	m_Texture.BindCubeMap();
	m_Shader.SetUniform<int>("u_CubeMap", { slot });
	m_Shader.Disable();
}

void Object::UnbindCubeMap() const
{
	m_Texture.UnbindCubeMap();
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
		m_TextCoords.push_back(temp.x * m_Texture.GetTileFactor());
		m_TextCoords.push_back(temp.y * m_Texture.GetTileFactor());;
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
		m_VAO = std::make_shared<OPENGL::VertexArray>();
		m_VAO->AddBuffer(std::make_shared<OPENGL::VertexBuffer>(&m_Vertices[0], m_Vertices.size(), 3), GEAR_BUFFER_POSITIONS);
		m_VAO->AddBuffer(std::make_shared<OPENGL::VertexBuffer>(&m_TextCoords[0], m_TextCoords.size(), 2), GEAR_BUFFER_TEXTCOORDS);
		m_VAO->AddBuffer(std::make_shared<OPENGL::VertexBuffer>(&m_Normals[0], m_Normals.size(), 3), GEAR_BUFFER_NORMALS);

		m_IBO = std::make_shared<OPENGL::IndexBuffer>(&m_Indices[0], m_Indices.size());
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::CROSSPLATFORM::Object: Unable to construct VAO, VBO, IBO! All data arrays have size = 0!" << std::endl;
	}
}