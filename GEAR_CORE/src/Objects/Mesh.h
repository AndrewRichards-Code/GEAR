#pragma once
#include "gear_core_common.h"
#include "Objects/ObjectInterface.h"
#include "Utils/ModelLoader.h"

namespace gear 
{
	namespace graphics
	{
		class Vertexbuffer;
		class Indexbuffer;
	}
	namespace objects
	{
		class GEAR_API Mesh : public ObjectComponentInterface
		{
		public:
			enum class VertexAttributes : uint32_t
			{
				NONE = 0x00,
				POSITIONS = 0x01,
				TEXCOORDS = 0x02,
				NORMALS = 0x04,
				TANGENTS = 0x08,
				BINORMALS = 0x10,
				COLOURS = 0x20
			};

		public:
			struct CreateInfo : public ObjectComponentInterface::CreateInfo
			{
				std::string						filepath;	//Option 1
				utils::ModelLoader::ModelData	modelData;	//Option 2
				VertexAttributes				vertexAttributes;
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

			void Update() override;

		protected:
			bool CreateInfoHasChanged(const ObjectComponentInterface::CreateInfo* pCreateInfo) override;

		public:
			inline const std::vector<Ref<graphics::Vertexbuffer>>& GetVertexBuffers() const { return m_VBs; }
			inline const std::vector<Ref<graphics::Indexbuffer>>& GetIndexBuffers() const { return m_IBs; }
			inline const std::vector<Ref<objects::Material>>& GetMaterials() const { return m_Materials; }
			inline std::vector<Ref<objects::Material>>& GetMaterials() { return m_Materials; }
			inline const utils::ModelLoader::ModelData& GetModelData() const { return m_CI.modelData; }
			inline utils::ModelLoader::ModelData& GetModelData() { return m_CI.modelData; }

			inline void SetOverrideMaterial(size_t index, const Ref<objects::Material>& material) { m_Materials[index] = material; }
			inline Ref<objects::Material>& GetMaterial(size_t index) { return m_Materials[index]; }
		};
	}
}
