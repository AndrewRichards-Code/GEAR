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
			Ref<Mesh>		pMesh;
			mars::Vec2			materialTextureScaling = mars::Vec2(1.0f, 1.0f);
			Transform			transform;
			std::string			renderPipelineName;
		};
	
	private:
		typedef graphics::UniformBufferStructures::Model ModelUB;
		Ref<graphics::Uniformbuffer<ModelUB>> m_UB;
	
	public:
		CreateInfo m_CI;
	
	public:
		Model(CreateInfo* pCreateInfo);
		~Model();
	
		//Update the skybox from the current state of Model::CreateInfo m_CI.
		void Update();
	
		inline const Ref<objects::Mesh>& GetMesh() const { return m_CI.pMesh; }
		inline const std::string& GetPipelineName() const { return m_CI.renderPipelineName; }
	
		inline Ref<graphics::Uniformbuffer<ModelUB>>& GetUB() { return m_UB; }
		inline const Ref<graphics::Uniformbuffer<ModelUB>>& GetUB() const { return m_UB; }
		inline const mars::Mat4& GetModlMatrix() const { return m_UB->modl; }
	
		inline std::string GetDebugName() const { return "GEAR_CORE_Model: " + m_CI.debugName; }
	
	private:
		void InitialiseUB();
	};
}
}