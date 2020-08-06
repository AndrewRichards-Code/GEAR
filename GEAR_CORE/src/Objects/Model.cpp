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
	m_DebugName = std::string("GEAR_CORE_Model: ") +  m_CI.debugName;
	
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

	m_UB->modlMatrix = 
		Mat4::Translation(m_CI.transform.translation) * 
		Quat::ToMat4(m_CI.transform.orientation) * 
		Mat4::Scale(m_CI.transform.scale);
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
	ubCI.debugName = m_DebugName.c_str();
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_UB = gear::CreateRef<Uniformbuffer<ModelUB>>(&ubCI);
}