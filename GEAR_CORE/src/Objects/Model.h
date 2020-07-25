#pragma once

#include "gear_core_common.h"
#include "Mesh.h"
#include "Graphics/Uniformbuffer.h"
#include "Graphics/Texture.h"
#include "Utils/FileUtils.h"
#include "mars.h"

namespace gear {
namespace objects {

struct Transform
{
	mars::Vec3 translation;
	mars::Quat orientation;
	mars::Vec3 scale;
};

class Model
{
public:
	struct CreateInfo
	{
		const char*						debugName;
		void*							device;
		gear::Ref<Mesh>					pMesh;
		Transform						transform;
		gear::Ref<graphics::Texture>	pTexture;
		mars::Vec4						colour;
		std::string						renderPipelineName;
	};

private:
	CreateInfo m_CI;

	struct ModelUB
	{
		mars::Mat4 modlMatrix;
		mars::Vec2 texCoordsScale0;
		mars::Vec2 texCoordsScale1;
	};
	gear::Ref<graphics::Uniformbuffer<ModelUB>> m_UB;

	std::string m_DebugName;

private:
	void InitialiseUB();
	void AddTextureIDsVB();
	void AddColourVB();

public:
	Model(CreateInfo* pCreateInfo);
	~Model();

	void SetUniformModlMatrix();
	void SetUniformModlMatrix(const mars::Mat4& modl);

	inline const std::map<Mesh::VertexBufferContents, gear::Ref<graphics::Vertexbuffer>> GetVBs() const { return m_CI.pMesh->GetVertexBuffers(); }
	inline const gear::Ref<graphics::Indexbuffer> GetIB() const { return m_CI.pMesh->GetIndexBuffer(); }
	inline const std::string& GetPipelineName() const { return m_CI.renderPipelineName; }
	inline const gear::Ref<graphics::Texture> GetTexture() const { return m_CI.pTexture; }
	inline const gear::Ref<graphics::Uniformbuffer<ModelUB>> GetUB() const { return m_UB; }
	inline const mars::Mat4 GetModlMatrix() const { return m_UB->modlMatrix; }
};
}
}