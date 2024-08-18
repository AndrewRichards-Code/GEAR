#include "gear_core_common.h"
#include "Objects/Mesh.h"
#include "Objects/Material.h"
#include "Graphics/Vertexbuffer.h"
#include "Graphics/Indexbuffer.h"

using namespace gear;
using namespace objects;
using namespace utils;

using namespace miru;
using namespace base;

Mesh::Mesh(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	Ref<ModelLoader::ModelData>& modelData = m_CI.modelData;

	graphics::Vertexbuffer::CreateInfo vbCI;
	vbCI.debugName = "GEAR_CORE_Mesh: " + m_CI.debugName;
	vbCI.device = m_CI.device;
	vbCI.stride = ModelLoader::GetSizeOfVertex();

	graphics::Indexbuffer::CreateInfo ibCI;
	ibCI.debugName = "GEAR_CORE_Mesh: " + m_CI.debugName;
	ibCI.device = m_CI.device;
	ibCI.stride = ModelLoader::GetSizeOfIndex();
	
	for (auto& mesh : modelData->meshes)
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
	m_VBs.clear();
	m_IBs.clear();
	m_Materials.clear();
}

void Mesh::Update()
{
	if (CreateInfoHasChanged(&m_CI))
	{
		uint64_t newHash = m_CreateInfoHash;
		*this = Mesh(&m_CI);
		m_CreateInfoHash = newHash; //Set the Hash value from the previous instance of the Mesh.
	}

	for (auto& material : m_Materials)
		material->Update();
}

bool Mesh::CreateInfoHasChanged(const ObjectComponentInterface::CreateInfo* pCreateInfo)
{
	const CreateInfo& CI = *reinterpret_cast<const CreateInfo*>(pCreateInfo);
	uint64_t newHash = 0;
	newHash ^= core::GetHash(CI.modelData);
	return CompareCreateInfoHash(newHash);
}
