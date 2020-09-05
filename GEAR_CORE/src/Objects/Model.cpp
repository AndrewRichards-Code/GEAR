#include "gear_core_common.h"
#include "Model.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

using namespace miru;
using namespace miru::crossplatform;

Model::Model(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	
	InitialiseUB();
	SetUniformModlMatrix();
}

Model::~Model()
{
}

void Model::SetUniformModlMatrix()
{
	m_UB->texCoordsScale0.x = m_CI.materialTextureScaling.x;
	m_UB->texCoordsScale0.y = m_CI.materialTextureScaling.y;
	m_UB->texCoordsScale1.x = m_CI.materialTextureScaling.x;
	m_UB->texCoordsScale1.y = m_CI.materialTextureScaling.y;

	m_UB->modlMatrix = TransformToMat4(m_CI.transform);
	m_UB->SubmitData();
}

void Model::SetUniformModlMatrix(const Mat4& modl)
{
	m_UB->modlMatrix = modl;
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