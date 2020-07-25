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
	AddTextureIDsVB();
	AddColourVB();
}

Model::~Model()
{
}

void Model::SetUniformModlMatrix()
{
	m_UB->texCoordsScale0.x = m_CI.pTexture->GetTileFactor();
	m_UB->texCoordsScale0.y = m_CI.pTexture->GetTileFactor();
	m_UB->texCoordsScale1.x = m_CI.pTexture->GetTileFactor();
	m_UB->texCoordsScale1.y = m_CI.pTexture->GetTileFactor();

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

	Uniformbuffer<ModelUB>::CreateInfo ubCI;
	ubCI.debugName = m_DebugName.c_str();
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_UB = gear::CreateRef<Uniformbuffer<ModelUB>>(&ubCI);
}

void Model::AddTextureIDsVB()
{
	std::vector<float> texIdData;
	const size_t& texIdCount = m_CI.pMesh->GetObjData().m_Vertices.size();
	texIdData.reserve(texIdCount);
	for (size_t i = 0; i < texIdCount; i++)
		texIdData.push_back(0.0f);

	Vertexbuffer::CreateInfo vbCI;
	vbCI.debugName = m_DebugName.c_str();
	vbCI.device = m_CI.device;
	vbCI.data = texIdData.data();
	vbCI.size = texIdData.size() * graphics::Vertexbuffer::GetVertexTypeSize(VertexType::FLOAT);
	vbCI.type = VertexType::FLOAT;

	gear::Ref<Vertexbuffer> texIdVB = gear::CreateRef<Vertexbuffer>(&vbCI);
	m_CI.pMesh->AddVertexBuffer(Mesh::VertexBufferContents::TEXTURE_ID, texIdVB);
}

void Model::AddColourVB()
{
	std::vector<Vec4> colourData;
	const size_t& colourDataCount = m_CI.pMesh->GetObjData().m_Vertices.size();
	colourData.reserve(colourDataCount);
	for (size_t i = 0; i < colourDataCount; i++)
		colourData.push_back(m_CI.colour);

	Vertexbuffer::CreateInfo vbCI;
	vbCI.debugName = m_DebugName.c_str();
	vbCI.device = m_CI.device;
	vbCI.data = colourData.data();
	vbCI.size = colourData.size() * graphics::Vertexbuffer::GetVertexTypeSize(VertexType::VEC4);
	vbCI.type = VertexType::VEC4;

	gear::Ref<Vertexbuffer> colourVB = gear::CreateRef<Vertexbuffer>(&vbCI);
	m_CI.pMesh->AddVertexBuffer(Mesh::VertexBufferContents::COLOUR, colourVB);
}
