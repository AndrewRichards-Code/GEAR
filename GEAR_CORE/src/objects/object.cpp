#include "object.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OBJECTS;
using namespace mars;

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
	m_ObjData.m_Vertices = { Vec4(-1, -1, 0, 1), Vec4(1, -1, 0, 1), Vec4(1, 1, 0, 1), Vec4(-1, 1, 0, 1) };
	m_ObjData.m_TexCoords = { Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1) };
	m_ObjData.m_Normals = { Vec4(0, 0, 1, 0) };
	m_ObjData.m_VertIndices = { 0, 1, 2, 2, 3, 0 };
	m_ObjData.m_TextIndices = { 0, 1, 2, 2, 3, 0 };
	m_ObjData.m_NormIndices = { 0, 1, 2, 2, 3, 0 };
	m_ObjData.m_UniqueVertices = { Vec3(0, 0, 0), Vec3(1, 1, 0), Vec3(2, 2, 0), Vec3(3, 3, 0) };

	m_Vertices.reserve(m_ObjData.GetSizeVertices() * 3);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec4 temp = m_ObjData.m_Vertices[(unsigned int)m_ObjData.m_UniqueVertices[i].x];
		temp.x *= size.x;
		temp.y *= size.y;
		temp = temp + Vec4(m_Position, 1.0f);
		m_Vertices.push_back(temp.x);
		m_Vertices.push_back(temp.y);
		m_Vertices.push_back(temp.z);
		m_Vertices.push_back(temp.w);
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
		Vec4 temp = m_ObjData.m_Normals[(unsigned int)m_ObjData.m_UniqueVertices[i].z];
		m_Normals.push_back(temp.x);
		m_Normals.push_back(temp.y);
		m_Normals.push_back(temp.z);
		m_Normals.push_back(temp.w);
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
	m_UBO->SubmitData(&m_ModelUBO.m_ModlMatrix.a, sizeof(ModelUBO));
}

void Object::SetUniformModlMatrix(const Mat4& modl)
{
	m_ModlMatrix = modl;
	m_ModelUBO.m_ModlMatrix = modl;
	m_UBO->SubmitData(&m_ModelUBO.m_ModlMatrix.a, sizeof(ModelUBO));
}

void Object::InitialiseUBO()
{
	m_UBO = std::make_shared<UniformBuffer>(m_Device, sizeof(ModelUBO), 1);

	const float zero[sizeof(ModelUBO)] = { 0 };
	m_UBO->SubmitData(zero, sizeof(ModelUBO));
}

void Object::LoadObjDataIntoObject()
{
	m_ObjData = FileUtils::read_obj(m_ObjFilePath);

	m_Vertices.reserve(m_ObjData.GetSizeVertices() * 4);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec4 temp = m_ObjData.m_Vertices[(unsigned int)m_ObjData.m_UniqueVertices[i].x];
		m_Vertices.push_back(temp.x);
		m_Vertices.push_back(temp.y);
		m_Vertices.push_back(temp.z);
		m_Vertices.push_back(temp.w);
	}

	m_TextCoords.reserve(m_ObjData.GetSizeVertices() * 2);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec2 temp = m_ObjData.m_TexCoords[(unsigned int)m_ObjData.m_UniqueVertices[i].y];
		m_TextCoords.push_back(temp.x * m_Texture->GetTileFactor());
		m_TextCoords.push_back(temp.y * m_Texture->GetTileFactor());
	}
	
	m_TextID.reserve(m_ObjData.GetSizeVertices());
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		m_TextID.push_back(0.0f);
	}

	m_Normals.reserve(m_ObjData.GetSizeVertices() * 4);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		Vec4 temp = m_ObjData.m_Normals[(unsigned int)m_ObjData.m_UniqueVertices[i].z];
		m_Normals.push_back(temp.x);
		m_Normals.push_back(temp.y);
		m_Normals.push_back(temp.z);
		m_Normals.push_back(temp.w);
	}

	m_Colours.reserve(m_ObjData.GetSizeVertices() * 4);
	for (int i = 0; i < m_ObjData.GetSizeVertices(); i++)
	{
		m_Colours.push_back(1.0f);
		m_Colours.push_back(1.0f);
		m_Colours.push_back(1.0f);
		m_Colours.push_back(1.0f);
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
		m_VBOs.emplace_back(std::make_shared<GRAPHICS::VertexBuffer>(m_Device, m_Vertices.data(), static_cast<unsigned int>(m_Vertices.size()), 4));
		m_VBOs.emplace_back(std::make_shared<GRAPHICS::VertexBuffer>(m_Device, m_TextCoords.data(), static_cast<unsigned int>(m_TextCoords.size()), 2));
		m_VBOs.emplace_back(std::make_shared<GRAPHICS::VertexBuffer>(m_Device, m_TextID.data(), static_cast<unsigned int>(m_TextID.size()), 1));
		m_VBOs.emplace_back(std::make_shared<GRAPHICS::VertexBuffer>(m_Device, m_Normals.data(), static_cast<unsigned int>(m_Normals.size()), 4));
		m_VBOs.emplace_back(std::make_shared<GRAPHICS::VertexBuffer>(m_Device, m_Colours.data(), static_cast<unsigned int>(m_Colours.size()), 4));

		m_IBO = std::make_shared<GRAPHICS::IndexBuffer>(m_Device, m_Indices.data(), static_cast<unsigned int>(m_Indices.size()));
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OBJECTS::Object: Unable to construct VBO, IBO. All data arrays have size = 0." << std::endl;
	}
}