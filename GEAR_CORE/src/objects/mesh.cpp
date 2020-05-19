#include "mesh.h"

using namespace GEAR;
using namespace OBJECTS;

Mesh::Mesh(void* device, const char* filepath)
	:m_Device(device), m_Filepath(filepath), m_Data(FileUtils::read_obj(filepath))
{
	m_VBs[VertexBufferContents::POSITION]		= std::make_shared<GRAPHICS::VertexBuffer>(m_Device, (float*)m_Data.m_Vertices.data(), m_Data.GetSizeVertices(), 4);
	m_VBs[VertexBufferContents::TEXTURE_COORD]	= std::make_shared<GRAPHICS::VertexBuffer>(m_Device, (float*)m_Data.m_TexCoords.data(), m_Data.GetSizeTexCoords(), 2);
	m_VBs[VertexBufferContents::NORMAL]			= std::make_shared<GRAPHICS::VertexBuffer>(m_Device, (float*)m_Data.m_Normals.data(), m_Data.GetSizeNormals(), 4);
	
	m_IB = std::make_shared<GRAPHICS::IndexBuffer>(m_Device, m_Data.m_VertIndices.data(), m_Data.GetSizeVertIndices());
}

Mesh::~Mesh()
{
}
