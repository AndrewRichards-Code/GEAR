#include "gear_core_common.h"
#include "Objects/Model.h"
#include "Objects/Mesh.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

Model::Model(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	
	InitialiseUB();
	Update(Transform());
}

Model::~Model()
{
}

void Model::Update(const Transform& transform)
{
	if (CreateInfoHasChanged(&m_CI))
	{
		m_UB->texCoordScale0.x = m_CI.materialTextureScaling.x;
		m_UB->texCoordScale0.y = m_CI.materialTextureScaling.y;
		m_UB->texCoordScale1.x = m_CI.materialTextureScaling.x;
		m_UB->texCoordScale1.y = m_CI.materialTextureScaling.y;
	}
	if (TransformHasChanged(transform))
	{
		m_UB->modl = TransformToMatrix4(transform);
	}
	if (m_UpdateGPU)
	{
		m_UB->SubmitData();
	}

	m_CI.pMesh->Update();
}

bool Model::CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo)
{
	const CreateInfo& CI = *reinterpret_cast<const CreateInfo*>(pCreateInfo);
	uint64_t newHash = 0;
	newHash ^= core::GetHash((uint64_t)CI.pMesh.get());
	newHash ^= core::GetHash(CI.materialTextureScaling.x);
	newHash ^= core::GetHash(CI.materialTextureScaling.x);
	newHash ^= core::GetHash(CI.renderPipelineName);
	return CompareCreateInfoHash(newHash);
}

void Model::InitialiseUB()
{
	float zero[sizeof(ModelUB)] = { 0 };

	Uniformbuffer<ModelUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Model: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_UB = CreateRef<Uniformbuffer<ModelUB>>(&ubCI);
}