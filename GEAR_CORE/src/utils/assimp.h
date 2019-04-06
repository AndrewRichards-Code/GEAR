#pragma once
#ifdef _M_X64
#include "gear_common.h"

namespace GEAR{
struct Mesh;
class Gear_Assimp
{
public:
	static void LoadModel(const std::string& filepath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR: GEAR::Gear_Assimp::LoadModel: " << importer.GetErrorString() << std::endl;
			return;
		}
		ProcessNode(scene->mRootNode, scene);
	}
private:
	static void ProcessNode(aiNode* node, const aiScene* scene)
	{
		std::vector<Mesh> result;
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			//result.push_back(ProcessMesh(mesh, scene));

		}
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}
};
}
#endif