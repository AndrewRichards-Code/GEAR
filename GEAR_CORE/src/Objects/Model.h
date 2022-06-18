#pragma once
#include "gear_core_common.h"
#include "Objects/ObjectInterface.h"
#include "Graphics/Uniformbuffer.h"

namespace gear 
{
	namespace objects
	{
		class Mesh;

		class GEAR_API Model : public ObjectInterface
		{
		public:
			struct CreateInfo : public ObjectInterface::CreateInfo
			{
				Ref<Mesh>			pMesh;
				mars::float2		materialTextureScaling = mars::float2(1.0f, 1.0f);
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

			//Update the model from the current state of Model::CreateInfo m_CI.
			void Update(const Transform& transform) override;

		protected:
			bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) override;

		public:
			inline const Ref<objects::Mesh>& GetMesh() const { return m_CI.pMesh; }
			inline const std::string& GetPipelineName() const { return m_CI.renderPipelineName; }

			inline Ref<graphics::Uniformbuffer<ModelUB>>& GetUB() { return m_UB; }
			inline const Ref<graphics::Uniformbuffer<ModelUB>>& GetUB() const { return m_UB; }
			inline const mars::float4x4& GetModlMatrix() const { return m_UB->modl; }

			inline std::string GetDebugName() const { return "GEAR_CORE_Model: " + m_CI.debugName; }

		private:
			void InitialiseUB();
		};
	}
}