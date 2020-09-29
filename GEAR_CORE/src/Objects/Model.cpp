#include "gear_core_common.h"
#include "Model.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

Model::Model(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	
	InitialiseUB();
	Update();
}

Model::~Model()
{
}

void Model::Update()
{
	m_UB->texCoordScale0.x = m_CI.materialTextureScaling.x;
	m_UB->texCoordScale0.y = m_CI.materialTextureScaling.y;
	m_UB->texCoordScale1.x = m_CI.materialTextureScaling.x;
	m_UB->texCoordScale1.y = m_CI.materialTextureScaling.y;

	m_UB->modl = TransformToMat4(m_CI.transform);
	m_UB->SubmitData();
}

void Model::InitialiseUB()
{
	float zero[sizeof(ModelUB)] = { 0 };

	Uniformbuffer<ModelUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Model: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_UB = gear::CreateRef<Uniformbuffer<ModelUB>>(&ubCI);
}