#include "gear_core_common.h"
#include "Mesh.h"

using namespace gear;
using namespace objects;

using namespace miru;
using namespace miru::crossplatform;

Mesh::Mesh(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	ModelLoader::SetDevice(m_CI.device);
	
	if(!m_CI.filepath.empty())
		m_CI.data = ModelLoader::LoadModelData(m_CI.filepath);

	graphics::Vertexbuffer::CreateInfo vbCI;
	vbCI.debugName = "GEAR_CORE_Mesh: " + m_CI.debugName;
	vbCI.device = m_CI.device;
	vbCI.stride = ModelLoader::GetSizeOfVertex();

	graphics::Indexbuffer::CreateInfo ibCI;
	ibCI.debugName = "GEAR_CORE_Mesh: " + m_CI.debugName;
	ibCI.device = m_CI.device;
	ibCI.stride = ModelLoader::GetSizeOfIndex();
	
	for (auto& mesh : m_CI.data.meshes)
	{
		vbCI.data = mesh.vertices.data();
		vbCI.size = mesh.vertices.size() * ModelLoader::GetSizeOfVertex();
		m_VBs.emplace_back(CreateRef<graphics::Vertexbuffer>(&vbCI));

		ibCI.data = mesh.indices.data();
		ibCI.size = mesh.indices.size() * ModelLoader::GetSizeOfIndex();
		m_IBs.emplace_back(CreateRef<graphics::Indexbuffer>(&ibCI));

		m_Materials.push_back(mesh.pMaterial);
	}
}

Mesh::~Mesh()
{
}