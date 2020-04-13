#include "object.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OBJECTS;
using namespace mars;

bool Object::s_InitialiseUBO = false;

Object::Object(void* device, const char* objFilePath, std::shared_ptr<GRAPHICS::Pipeline> pipeline, std::shared_ptr<GRAPHICS::Texture> texture, const mars::Mat4& modl)
	:m_Device(device), m_ObjFilePath(objFilePath), m_Pipeline(pipeline), m_Texture(texture), m_Colour({0.0f, 0.0f, 0.0f, 0.0f}), m_ModlMatrix(modl)
{
	InitialiseUBO();
	LoadObjDataIntoObject();
	GenVBOandIBO();
	SetUniformModlMatrix();
}

Object::Object(void* device, const char* objFilePath, std::shared_ptr<GRAPHICS::Pipeline> pipeline, const mars::Vec4& colour, const mars::Mat4& modl)
	:m_Device(device), m_ObjFilePath(objFilePath), m_Pipeline(pipeline), m_Colour(colour), m_ModlMatrix(modl)
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
	GenVBOandIBO();
	SetUniformModlMatrix();
}

Object::Object(void* device, const char* objFilePath, std::shared_ptr<GRAPHICS::Pipeline> pipeline, std::shared_ptr<GRAPHICS::Texture> texture, const mars::Vec4& colour, const mars::Vec3& position, const mars::Vec2& size)
	:m_Device(device), m_ObjFilePath(objFilePath), m_Pipeline(pipeline), m_Texture(texture), m_Colour(colour), m_Position(position), m_Size(size)
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
		m_TextCoords.push_back(temp.x * m_Texture->GetTileFactor());
		m_TextCoords.push_back(temp.y * m_Texture->GetTileFactor());
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

void Object::SetUniformModlMatrix()
{
	m_ModelUBO.m_ModlMatrix = m_ModlMatrix;
	m_ModelUBO.m_ModlMatrix.Transpose();
	m_UBO->SubmitData(&m_ModelUBO.m_ModlMatrix.a, sizeof(Mat4), offsetof(ModelUBO, m_ModlMatrix));
}

void Object::SetUniformModlMatrix(const Mat4& modl)
{
	m_ModlMatrix = modl;
	m_ModelUBO.m_ModlMatrix = modl;
	m_ModelUBO.m_ModlMatrix.Transpose();
	m_UBO->SubmitData(&m_ModelUBO.m_ModlMatrix.a, sizeof(Mat4), offsetof(ModelUBO, m_ModlMatrix));
}

void Object::InitialiseUBO()
{
	if (s_InitialiseUBO == false)
	{
		m_UBO = std::make_shared<UniformBuffer>(m_Device, sizeof(ModelUBO), 1);
		s_InitialiseUBO = true;

		const float zero[sizeof(ModelUBO)] = { 0 };
		m_UBO->SubmitData(zero, sizeof(ModelUBO), 0);
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
		m_TextCoords.push_back(temp.x * m_Texture->GetTileFactor());
		m_TextCoords.push_back(temp.y * m_Texture->GetTileFactor());
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

void Object::GenVBOandIBO()
{
	if (m_Vertices.size() && m_TextCoords.size() && m_Normals.size() && m_Indices.size() > 0)
	{
		m_VBOs.emplace_back(std::make_shared<GRAPHICS::VertexBuffer>(m_Device, m_Vertices.data(), static_cast<unsigned int>(m_Vertices.size()), 3));
		m_VBOs.emplace_back(std::make_shared<GRAPHICS::VertexBuffer>(m_Device, m_TextCoords.data(), static_cast<unsigned int>(m_TextCoords.size()), 2));
		m_VBOs.emplace_back(std::make_shared<GRAPHICS::VertexBuffer>(m_Device, m_Normals.data(), static_cast<unsigned int>(m_Normals.size()), 3));

		m_IBO = std::make_shared<GRAPHICS::IndexBuffer>(m_Device, m_Indices.data(), static_cast<unsigned int>(m_Indices.size()));
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OBJECTS::Object: Unable to construct VAO, VBO, IBO! All data arrays have size = 0!" << std::endl;
	}
}