#pragma once
#include "gear_core_common.h"
#include "Animation/Animation.h"
#include "Objects/Material.h"

struct aiScene;
struct aiNode;
struct aiMaterial;

namespace gear 
{
	namespace animation
	{
		struct Animation;
	}
	namespace utils
	{
		class GEAR_API ModelLoader
		{
		public:
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
			struct Node
			{
				std::string			name;
				mars::float4x4		transform;
				size_t				meshIndex = ~0;
				size_t				animationIndex = ~0;
				size_t				nodeAnimationIndex = ~0;
				std::vector<Node>	children;
			};

			struct ModelData : public asset::Asset
			{
				std::vector<MeshData>				meshes;
				std::vector<animation::Animation>	animations;
				Node								nodeGraph;
			};

		public:
			static ModelData LoadModelData(const std::filesystem::path& filepath);

			inline static void SetDevice(void* device) { m_Device = device; }
			inline constexpr static size_t GetSizeOfVertex() { return sizeof(Vertex); }
			inline constexpr static size_t GetSizeOfIndex() { return sizeof(uint32_t); }

		private:
			static void BuildNodeGraph(const aiScene* scene, aiNode* node, Node& thisNode, ModelData& modelData);

			static std::vector<MeshData> ProcessMeshes(aiNode* node, const aiScene* scene);
			static std::vector<animation::Animation> ProcessAnimations(const aiScene* scene);

			static void FillOutMaterialCreateInfo(aiMaterial* aiMaterial, objects::Material::CreateInfo& materialCreateInfo);

		private:
			static void* m_Device;
		};
	}
}