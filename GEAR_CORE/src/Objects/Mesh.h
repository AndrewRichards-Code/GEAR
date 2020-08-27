#pragma once

#include "gear_core_common.h"
#include "Graphics/Vertexbuffer.h"
#include "Graphics/Indexbuffer.h"
#include "Utils/ModelLoader.h"

namespace gear 
{
namespace objects 
{
	class Mesh
	{
	public:
		struct CreateInfo
		{
			std::string debugName;
			void*		device;
			std::string filepath;
		};

	private:
		CreateInfo m_CI;
		ModelLoader::ModelData m_Data;

		std::vector<gear::Ref<graphics::Vertexbuffer>> m_VBs;
		std::vector<gear::Ref<graphics::Indexbuffer>> m_IBs;
		std::vector<gear::Ref<objects::Material>> m_Materials;

	public:
		Mesh(CreateInfo* pCreateInfo);
		~Mesh();

		inline const std::vector<gear::Ref<graphics::Vertexbuffer>>& GetVertexBuffers() const { return m_VBs; }
		inline const std::vector<gear::Ref<graphics::Indexbuffer>>& GetIndexBuffers() const { return m_IBs; }
		inline const std::vector<gear::Ref<objects::Material>>& GetMaterials() const { return m_Materials; }
		inline const ModelLoader::ModelData& GetModelData() const { return m_Data; }

		inline void SetOverrideMaterial(size_t index, const gear::Ref<objects::Material>& material) { m_Materials[index] = material; }
	};
}
}
