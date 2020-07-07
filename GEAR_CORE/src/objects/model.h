#pragma once

#include "gear_core_common.h"
#include "mesh.h"
#include "graphics/miru/buffer/uniformbuffer.h"
#include "graphics/miru/pipeline.h"
#include "graphics/miru/texture.h"
#include "utils/fileutils.h"
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
		void*							device;
		gear::Ref<Mesh>					pMesh;
		Transform						transform;
		gear::Ref<graphics::Texture>	pTexture;
		mars::Vec4						colour;
		gear::Ref<graphics::Pipeline>	pPipeline;
	};

private:
	CreateInfo m_CI;

	struct ModelUB
	{
		mars::Mat4 m_ModlMatrix;
	} m_ModelUB;
	gear::Ref<graphics::UniformBuffer> m_UB;


private:
	void InitialiseUB();
	void AddTextureIDsVB();
	void AddColourVB();

public:
	Model(CreateInfo* pCreateInfo);
	~Model();

	void SetUniformModlMatrix();
	void SetUniformModlMatrix(const mars::Mat4& modl);

	inline static void SetContext(miru::Ref<miru::crossplatform::Context> context)
	{
		graphics::VertexBuffer::SetContext(context);
		graphics::IndexBuffer::SetContext(context);
		graphics::UniformBuffer::SetContext(context);
		graphics::Texture::SetContext(context);
	};

	inline const std::map<Mesh::VertexBufferContents, gear::Ref<graphics::VertexBuffer>> GetVBs() const { return m_CI.pMesh->GetVertexBuffers(); }
	inline const gear::Ref<graphics::IndexBuffer> GetIB() const { return m_CI.pMesh->GetIndexBuffer(); }
	inline const gear::Ref<graphics::Pipeline> GetPipeline() const { return m_CI.pPipeline; }
	inline const gear::Ref<graphics::Texture> GetTexture() const { return m_CI.pTexture; }
	inline const gear::Ref<graphics::UniformBuffer> GetUB() const { return m_UB; }
	inline const mars::Mat4 GetModlMatrix() const { return m_ModelUB.m_ModlMatrix; }
};
}
}