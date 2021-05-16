#pragma once
#include "gear_core_common.h"

namespace gear 
{
	//Forward Declaration
	namespace objects
	{
		class Material;
		struct Transform;
	}
	
	class ModelLoader
	{
	public:
		struct Vertex
		{
			mars::Vec4 position;
			mars::Vec2 texCoord;
			mars::Vec4 normal;
			mars::Vec4 tangent;
			mars::Vec4 binormal;
			mars::Vec4 colour;
		};
		struct Bone
		{
			mars::Mat4								transform;
			std::vector<std::pair<uint32_t, float>> vertexIDsAndWeights;
		};
		struct NodeAnimation
		{
			typedef std::pair<double, objects::Transform> Keyframe;
			typedef std::vector<Keyframe> Keyframes;

			std::string		name;
			Keyframes		keyframes;

		};
		struct Animation 
		{
			double						duration;
			uint32_t					framesPerSecond;
			std::vector<NodeAnimation>	nodeAnimations;
		};
		struct MeshData
		{
			std::string				name;
			std::vector<Vertex>		vertices;
			std::vector<uint32_t>	indices;
			std::vector<Bone>		bones;
			Ref<objects::Material>	pMaterial;
		};

	private:
		static void* m_Device;
		static std::vector<MeshData> modelData;
		static std::vector<Animation> animationData;
	
	public:
		inline static void SetDevice(void* device) { m_Device = device; }
		inline constexpr static size_t GetSizeOfVertex() { return sizeof(Vertex); }
		inline constexpr static size_t GetSizeOfIndex() { return sizeof(uint32_t); }
	
		typedef std::vector<MeshData> ModelData;
		static ModelData LoadModelData(const std::string& filepath);
	
	private:
		static void ProcessNode(aiNode* node, const aiScene* scene);
		static MeshData ProcessMesh(aiMesh* mesh, aiNode* node, const aiScene* scene);
		static std::vector<std::string> GetMaterialFilePath(aiMaterial* material, aiTextureType type);
		static void AddMaterialProperties(aiMaterial* aiMaterial, Ref<objects::Material> material);
		static void ProcessAnimations(const aiScene* scene);
	};
}