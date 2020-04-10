#include "object.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace CROSSPLATFORM;
using namespace mars;

bool Object::s_InitialiseUBO = false;

Object::Object(const char* objFilePath, OPENGL::Shader& shader, const OPENGL::Texture& texture, const Mat4& modl)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Texture(texture), m_Colour({0.0f, 0.0f, 0.0f, 0.0f}), m_ModlMatrix(modl)
{
	InitialiseUBO();
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

Object::Object(const char* objFilePath, OPENGL::Shader& shader, const mars::Vec4& colour, const mars::Mat4& modl)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Colour(colour), m_ModlMatrix(modl)
{
	InitialiseUBO();
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
	m_VAO->AddBuffer(std::make_shared<OPENGL::VertexBuffer>(m_Colours.data(), static_cast<unsigned int>(m_Colours.size()), 4), OPENGL::VertexArray::BufferType::GEAR_BUFFER_COLOURS);
	SetUniformModlMatrix();
}

Object::Object(const char* objFilePath, OPENGL::Shader& shader, const OPENGL::Texture& texture, const Vec4& colour, const Vec3& position, const Vec2& size)
	:m_ObjFilePath(objFilePath), m_Shader(shader), m_Texture(texture), m_Colour(colour), m_Position(position), m_Size(size)
{
	InitialiseUBO();
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
		m_TextCoords.push_back(temp.y * m_Texture.GetTileFactor());
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

	m_Colours.resize(2 * m_TextCoords.size());
	for (int i = 0; i < (int)m_Colours.size();)
	{
		m_Colours[i + 0] = m_Colour.r;
		m_Colours[i + 1] = m_Colour.g;
		m_Colours[i + 2] = m_Colour.b;
		m_Colours[i + 3] = m_Colour.a;
		i += 4;
	}

	SetUniformModlMatrix(Mat4::Identity());
	b_ForBatchRenderer2D = true;
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

void Object::SetUniformModlMatrix()
{
	m_ModelUBO.m_ModlMatrix = m_ModlMatrix;
	m_ModelUBO.m_ModlMatrix.Transpose();
	OPENGL::BufferManager::UpdateUBO(1, &m_ModelUBO.m_ModlMatrix.a, sizeof(Mat4), offsetof(ModelUBO, m_ModlMatrix));
}

void Object::SetUniformModlMatrix(const Mat4& modl)
{
	m_ModlMatrix = modl;
	m_ModelUBO.m_ModlMatrix = modl;
	m_ModelUBO.m_ModlMatrix.Transpose();
	OPENGL::BufferManager::UpdateUBO(1, &m_ModelUBO.m_ModlMatrix.a, sizeof(Mat4), offsetof(ModelUBO, m_ModlMatrix));
}

void Object::InitialiseUBO()
{
	if (s_InitialiseUBO == false)
	{
		OPENGL::BufferManager::AddUBO(sizeof(ModelUBO), 1);
		s_InitialiseUBO = true;

		const float zero[sizeof(ModelUBO)] = { 0 };
		OPENGL::BufferManager::UpdateUBO(1, zero, sizeof(ModelUBO), 0);
	}
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
		m_TextCoords.push_back(temp.y * m_Texture.GetTileFactor());
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
		m_VAO->AddBuffer(std::make_shared<OPENGL::VertexBuffer>(m_Vertices.data(), static_cast<unsigned int>(m_Vertices.size()), 3), OPENGL::VertexArray::BufferType::GEAR_BUFFER_POSITIONS);
		m_VAO->AddBuffer(std::make_shared<OPENGL::VertexBuffer>(m_TextCoords.data(), static_cast<unsigned int>(m_TextCoords.size()), 2), OPENGL::VertexArray::BufferType::GEAR_BUFFER_TEXCOORDS);
		m_VAO->AddBuffer(std::make_shared<OPENGL::VertexBuffer>(m_Normals.data(), static_cast<unsigned int>(m_Normals.size()), 3), OPENGL::VertexArray::BufferType::GEAR_BUFFER_NORMALS);

		m_IBO = std::make_shared<OPENGL::IndexBuffer>(m_Indices.data(), static_cast<unsigned int>(m_Indices.size()));
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::CROSSPLATFORM::Object: Unable to construct VAO, VBO, IBO! All data arrays have size = 0!" << std::endl;
	}
}