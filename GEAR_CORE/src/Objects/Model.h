#pragma once

#include "gear_core_common.h"
#include "Mesh.h"
#include "Material.h"
#include "Graphics/Uniformbuffer.h"
#include "Utils/FileUtils.h"

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
		const char*			debugName;
		void*				device;
		gear::Ref<Mesh>		pMesh;
		mars::Vec2			materialTextureScaling = mars::Vec2(1.0f, 1.0f);
		Transform			transform;
		std::string			renderPipelineName;
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

public:
	Model(CreateInfo* pCreateInfo);
	~Model();

	void SetUniformModlMatrix();
	void SetUniformModlMatrix(const mars::Mat4& modl);

	inline const gear::Ref<objects::Mesh>& GetMesh() const { return m_CI.pMesh; }
	inline const std::string& GetPipelineName() const { return m_CI.renderPipelineName; }

	inline const gear::Ref<graphics::Uniformbuffer<ModelUB>> GetUB() const { return m_UB; }
	inline const mars::Mat4 GetModlMatrix() const { return m_UB->modlMatrix; }
	
	inline const std::string& GetDebugName() const { return m_DebugName; }
};
}
}