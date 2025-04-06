#pragma once
#include "gear_core_common.h"
#include "Asset/Asset.h"
#include "Objects/ObjectInterface.h"

namespace gear 
{
	namespace animation
	{
		struct Animation;
	}
	namespace graphics
	{
		class Vertexbuffer;
		class Indexbuffer;
	}
	namespace objects
	{
		class Material;

		struct Vertex
		{
			mars::float4 position;
			mars::float2 texCoord;
			mars::float4 normal;
			mars::float4 tangent;
			mars::float4 binormal;
			mars::float4 colour;
		};
		struct Bone
		{
			mars::float4x4							transform;
			std::vector<std::pair<uint32_t, float>> vertexIDsAndWeights;
		};

		struct MeshData
		{
			std::string				meshName;
			std::string				nodeName;
			std::vector<Vertex>		vertices;
			std::vector<uint32_t>	indices;
			std::vector<Bone>		bones;
			Ref<objects::Material>	pMaterial;
		};
		struct MeshNode
		{
			std::string				name;
			mars::float4x4			transform;
			size_t					meshIndex = ~0;
			size_t					animationIndex = ~0;
			size_t					nodeAnimationIndex = ~0;
			std::vector<MeshNode>	children;
		};

		struct ModelData : public asset::Asset
		{
			std::vector<MeshData>				meshes;
			std::vector<animation::Animation>	animations;
			MeshNode								nodeGraph;
		};

		class GEAR_OBJECTS_API Mesh : public ObjectComponentInterface, public asset::Asset
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
				Ref<ModelData>		modelData;
				VertexAttributes	vertexAttributes;
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
			inline const ModelData& GetModelData() const { return *m_CI.modelData; }
			inline ModelData& GetModelData() { return *m_CI.modelData; }

			inline void SetOverrideMaterial(size_t index, const Ref<objects::Material>& material) { m_Materials[index] = material; }
			inline Ref<objects::Material>& GetMaterial(size_t index) { return m_Materials[index]; }
		};
	}
}
