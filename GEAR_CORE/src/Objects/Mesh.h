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
			std::string				debugName;
			void*					device;
			std::string				filepath;
			ModelLoader::ModelData	data;
		};

	private:
		std::vector<Ref<graphics::Vertexbuffer>> m_VBs;
		std::vector<Ref<graphics::Indexbuffer>> m_IBs;
		std::vector<Ref<objects::Material>> m_Materials;

	public:
		CreateInfo m_CI;

	public:
		Mesh(CreateInfo* pCreateInfo);
		~Mesh();

		inline const std::vector<Ref<graphics::Vertexbuffer>>& GetVertexBuffers() const { return m_VBs; }
		inline const std::vector<Ref<graphics::Indexbuffer>>& GetIndexBuffers() const { return m_IBs; }
		inline const std::vector<Ref<objects::Material>>& GetMaterials() const { return m_Materials; }
		inline std::vector<Ref<objects::Material>>& GetMaterials() { return m_Materials; }
		inline const ModelLoader::ModelData& GetModelData() const { return m_CI.data; }

		inline void SetOverrideMaterial(size_t index, const Ref<objects::Material>& material) { m_Materials[index] = material; }
		inline Ref<objects::Material>& GetMaterial(size_t index) { return m_Materials[index]; }
	};
}
}
