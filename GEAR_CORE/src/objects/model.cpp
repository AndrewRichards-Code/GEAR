#include "gear_core_common.h"
#include "model.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

Model::Model(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	m_DebugName = std::string("GEAR_CORE_Model: ") +  m_CI.debugName;

	InitialiseUB();
	SetUniformModlMatrix();
	AddTextureIDsVB();
	AddColourVB();
}

Model::~Model()
{
}

void Model::SetUniformModlMatrix()
{
	m_UB->modlMatrix = Mat4::
		Translation(m_CI.transform.translation)
		* Quat::ToMat4(m_CI.transform.orientation)
		* Mat4::Scale(m_CI.transform.scale);
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

	UniformBuffer<ModelUB>::CreateInfo ubCI;
	ubCI.debugName = m_DebugName.c_str();
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_UB = gear::CreateRef<UniformBuffer<ModelUB>>(&ubCI);
}

void Model::AddTextureIDsVB()
{
	std::vector<float> texIdData;
	const size_t& texIdCount = m_CI.pMesh->GetObjData().m_Vertices.size();
	texIdData.reserve(texIdCount);
	for (size_t i = 0; i < texIdCount; i++)
		texIdData.push_back(0.0f);

	VertexBuffer::CreateInfo vbCI;
	vbCI.debugName = m_DebugName.c_str();
	vbCI.device = m_CI.device;
	vbCI.data = texIdData.data();
	vbCI.size = texIdData.size() * graphics::VertexBuffer::GetVertexTypeSize(miru::crossplatform::VertexType::FLOAT);
	vbCI.type = miru::crossplatform::VertexType::FLOAT;

	gear::Ref<VertexBuffer> texIdVB = gear::CreateRef<VertexBuffer>(&vbCI);
	m_CI.pMesh->AddVertexBuffer(Mesh::VertexBufferContents::TEXTURE_ID, texIdVB);
}

void Model::AddColourVB()
{
	std::vector<Vec4> colourData;
	const size_t& colourDataCount = m_CI.pMesh->GetObjData().m_Vertices.size();
	colourData.reserve(colourDataCount);
	for (size_t i = 0; i < colourDataCount; i++)
		colourData.push_back(m_CI.colour);

	VertexBuffer::CreateInfo vbCI;
	vbCI.debugName = m_DebugName.c_str();
	vbCI.device = m_CI.device;
	vbCI.data = colourData.data();
	vbCI.size = colourData.size() * graphics::VertexBuffer::GetVertexTypeSize(miru::crossplatform::VertexType::VEC4);
	vbCI.type = miru::crossplatform::VertexType::VEC4;

	gear::Ref<VertexBuffer> colourVB = gear::CreateRef<VertexBuffer>(&vbCI);
	m_CI.pMesh->AddVertexBuffer(Mesh::VertexBufferContents::COLOUR, colourVB);
}
