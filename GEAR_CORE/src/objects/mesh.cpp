#include "mesh.h"

using namespace gear;
using namespace objects;

Mesh::Mesh(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_Data = FileUtils::read_obj(m_CI.filepath.c_str());

	graphics::VertexBuffer::CreateInfo vbCI;
	vbCI.device = m_CI.device;

	vbCI.data = m_Data.m_Vertices.data();
	vbCI.size = m_Data.m_Vertices.size() * graphics::VertexBuffer::GetVertexTypeSize(miru::crossplatform::VertexType::VEC4);
	vbCI.type = miru::crossplatform::VertexType::VEC4;
	m_VBs[VertexBufferContents::POSITION] = gear::CreateRef<graphics::VertexBuffer>(&vbCI);

	vbCI.data = m_Data.m_TexCoords.data();
	vbCI.size = m_Data.m_TexCoords.size() * graphics::VertexBuffer::GetVertexTypeSize(miru::crossplatform::VertexType::VEC2);
	vbCI.type = miru::crossplatform::VertexType::VEC2;
	m_VBs[VertexBufferContents::TEXTURE_COORD] = gear::CreateRef<graphics::VertexBuffer>(&vbCI);
	
	vbCI.data = m_Data.m_Normals.data();
	vbCI.size = m_Data.m_Normals.size() * graphics::VertexBuffer::GetVertexTypeSize(miru::crossplatform::VertexType::VEC4);
	vbCI.type = miru::crossplatform::VertexType::VEC4;
	m_VBs[VertexBufferContents::NORMAL] = gear::CreateRef<graphics::VertexBuffer>(&vbCI);
	
	graphics::IndexBuffer::CreateInfo ibCI;
	ibCI.device = m_CI.device;
	ibCI.data = m_Data.m_VertIndices.data();
	ibCI.size = m_Data.m_VertIndices.size() * sizeof(uint32_t);
	ibCI.stride = sizeof(uint32_t);
	m_IB = gear::CreateRef<graphics::IndexBuffer>(&ibCI);
}

Mesh::~Mesh()
{
}

void Mesh::AddVertexBuffer(VertexBufferContents type, gear::Ref<graphics::VertexBuffer> vertexBuffer)
{
	m_VBs[type] = vertexBuffer;
}

void Mesh::RemoveVertexBuffer(VertexBufferContents type)
{
	m_VBs.erase(type);
}
