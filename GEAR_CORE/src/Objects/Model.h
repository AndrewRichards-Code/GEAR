#pragma once

#include "gear_core_common.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Graphics/Uniformbuffer.h"
#include "Utils/FileUtils.h"

namespace gear 
{
namespace objects 
{
	class Model
	{
	public:
		struct CreateInfo
		{
			std::string			debugName;
			void*				device;
			gear::Ref<Mesh>		pMesh;
			mars::Vec2			materialTextureScaling = mars::Vec2(1.0f, 1.0f);
			Transform			transform;
			std::string			renderPipelineName;
		};
	
	private:
		struct ModelUB
		{
			mars::Mat4 modlMatrix;
			mars::Vec2 texCoordsScale0;
			mars::Vec2 texCoordsScale1;
		};
		gear::Ref<graphics::Uniformbuffer<ModelUB>> m_UB;
	
	public:
		CreateInfo m_CI;
	
	public:
		Model(CreateInfo* pCreateInfo);
		~Model();
	
		void SetUniformModlMatrix();
		void SetUniformModlMatrix(const mars::Mat4& modl);
	
		inline const gear::Ref<objects::Mesh>& GetMesh() const { return m_CI.pMesh; }
		inline const std::string& GetPipelineName() const { return m_CI.renderPipelineName; }
	
		inline const gear::Ref<graphics::Uniformbuffer<ModelUB>> GetUB() const { return m_UB; }
		inline const mars::Mat4 GetModlMatrix() const { return m_UB->modlMatrix; }
	
		inline std::string GetDebugName() const { return "GEAR_CORE_Model: " + m_CI.debugName; }
	
	private:
		void InitialiseUB();
	};
}
}