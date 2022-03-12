#pragma once
#include "gear_core_common.h"
#include "ModelLoader.h"
#include "Objects/Material.h"
#include "Objects/Transform.h"

using namespace gear;
using namespace graphics;
using namespace animation;
using namespace objects;

using namespace mars;

void* ModelLoader::m_Device = nullptr;

ModelLoader::ModelData ModelLoader::LoadModelData(const std::string& filepath)
{
	bool calculateTangentsAndBiNormals = true;
	bool flipUVs = true;

	Assimp::Importer importer;
	unsigned int flags = aiProcess_Triangulate /*| aiProcess_PreTransformVertices*/;
	if (calculateTangentsAndBiNormals)
		flags |= aiProcess_CalcTangentSpace;
	if (flipUVs)
		flags |= aiProcess_FlipUVs;
	const aiScene* scene = importer.ReadFile(filepath, flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		GEAR_WARN(ErrorCode::UTILS | ErrorCode::INIT_FAILED, "Assimp error: %s.", importer.GetErrorString());
		return ModelData();
	}

	double scale = 1.0;
	scene->mMetaData->Get("UnitScaleFactor", scale);

	ModelData modelData;
	BuildNodeGraph(scene, scene->mRootNode, modelData.nodeGraph, modelData);

	Material::ClearLoadedMaterials();

	return modelData;
}

void ModelLoader::BuildNodeGraph(const aiScene* scene, aiNode* node, Node& thisNode, ModelData& modelData)
{
	if (scene && node && modelData.animations.empty())
	{
		if (node == scene->mRootNode) //Only process animations at the root node.
		{
			modelData.animations = ProcessAnimations(scene);
		}
	}

	if (node)
	{
		thisNode.name = node->mName.C_Str();
		Convert_aiMatrix4x4ToFloat4x4(node->mTransformation, thisNode.transform);

		if(node->mNumMeshes > 0)
		{
			for (auto& mesh : ProcessMeshes(node, scene))
				modelData.meshes.push_back(mesh);
		}

		for (size_t i = 0; i < modelData.meshes.size(); i++)
		{
			const auto& mesh = modelData.meshes[i];
			if (mesh.nodeName.compare(thisNode.name) == 0)
				thisNode.meshIndex = i;
		}
		for (size_t i = 0; i < modelData.animations.size(); i++)
		{
			const auto& animation = modelData.animations[i];
			for (size_t j = 0; j < animation.nodeAnimations.size(); j++)
			{
				const auto& nodeAnimation = animation.nodeAnimations[i];
				if (nodeAnimation.name.compare(thisNode.name) == 0)
				{
					thisNode.animationIndex = i;
					thisNode.nodeAnimationIndex = j;
				}
			}
		}

		//Index children
		aiNode** children = node->mChildren;
		const unsigned int& childrenCount = node->mNumChildren;
		thisNode.children.resize((size_t)childrenCount);
		for (size_t i = 0; i < static_cast<size_t>(childrenCount); i++)
		{
			BuildNodeGraph(scene, children[i],  thisNode.children[i], modelData);
		}
	}
}

std::vector<ModelLoader::MeshData> ModelLoader::ProcessMeshes(aiNode* node, const aiScene* scene)
{
	std::vector<MeshData> meshes;
	meshes.reserve(node->mNumMeshes);

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		MeshData meshData;
		meshData.meshName = std::string(mesh->mName.C_Str());
		meshData.nodeName = std::string(node->mName.C_Str());

		//Vertices
		meshData.vertices.reserve(mesh->mNumVertices);
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex = {};
			if (mesh->HasPositions())
			{
				vertex.position = float4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
			}
			if (mesh->HasTextureCoords(0))
			{
				vertex.texCoord = float2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			}
			if (mesh->HasNormals())
			{
				vertex.normal = float4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0.0f);
			}
			if (mesh->HasTangentsAndBitangents())
			{
				vertex.tangent = float4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 0.0f);
				vertex.binormal = float4(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z, 0.0f);
			}
			if (mesh->HasVertexColors(0))
			{
				vertex.colour = float4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
			}
			meshData.vertices.push_back(vertex);
		}

		//Indices
		meshData.indices.reserve(mesh->mNumFaces * 3);
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				meshData.indices.push_back(face.mIndices[j]);
		}

		//Bones
		meshData.bones.reserve(mesh->mNumBones);
		for (unsigned int i = 0; i < mesh->mNumBones; i++)
		{
			meshData.bones.push_back({});

			float4x4& transform = meshData.bones.back().transform;
			aiMatrix4x4 aiTransform = mesh->mBones[i]->mOffsetMatrix;
			Convert_aiMatrix4x4ToFloat4x4(aiTransform, transform);

			std::vector<std::pair<uint32_t, float>>& vertexIDsAndWeights = meshData.bones.back().vertexIDsAndWeights;
			for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
			{
				const aiVertexWeight& weight = mesh->mBones[i]->mWeights[j];
				vertexIDsAndWeights.push_back({ weight.mVertexId, weight.mWeight });
			}
		}

		//Materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);

		Ref<Material> mat = Material::FindMaterial(materialName.C_Str());
		if (mat != nullptr)
		{
			meshData.pMaterial = mat;
		}
		else
		{
			Material::CreateInfo materialCI;
			materialCI.debugName = materialName.C_Str();
			materialCI.device = m_Device;
			FillOutMaterialCreateInfo(material, materialCI);
			meshData.pMaterial = CreateRef<Material>(&materialCI);

			Material::AddMaterial(materialName.C_Str(), meshData.pMaterial);
		}
		meshes.push_back(meshData);
	}
	return meshes;
}
std::vector<animation::Animation> ModelLoader::ProcessAnimations(const aiScene* scene)
{
	std::vector<animation::Animation> animations;
	animations.reserve(scene->mNumAnimations);

	for (unsigned int i = 0; i < scene->mNumAnimations; i++)
	{
		Animation animation;
		aiAnimation*& _animation = scene->mAnimations[i];

		animation.sequenceType = core::Sequence::Type::ANIMATION;
		animation.duration = _animation->mDuration;
		animation.framesPerSecond = static_cast<uint32_t>(_animation->mTicksPerSecond);
		animation.nodeAnimations.reserve(_animation->mNumChannels);

		for (unsigned int j = 0; j < _animation->mNumChannels; j++)
		{
			aiNodeAnim*& nodeAnim = _animation->mChannels[j];

			animation.nodeAnimations.push_back({});
			NodeAnimation& node = animation.nodeAnimations.back();
			node.name = std::string(nodeAnim->mNodeName.C_Str());
			NodeAnimation::Keyframes& keyframes = node.keyframes;

			if (node.name.find("Translation") != std::string::npos)
			{
				node.type = NodeAnimation::Type::TRANSLATION;
				keyframes.reserve(nodeAnim->mNumPositionKeys);
				for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++)
				{
					double timepoint = nodeAnim->mPositionKeys[k].mTime;
					float3 translation = float3(
						nodeAnim->mPositionKeys[k].mValue.x,
						nodeAnim->mPositionKeys[k].mValue.y,
						nodeAnim->mPositionKeys[k].mValue.z);

					Transform transform;
					transform.translation = translation;
					NodeAnimation::Keyframe kf = { timepoint , transform };
					keyframes.push_back(kf);
				}
			}
			else if (node.name.find("Rotation") != std::string::npos)
			{
				node.type = NodeAnimation::Type::ROTATION;
				keyframes.reserve(nodeAnim->mNumRotationKeys);
				for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; k++)
				{
					double timepoint = nodeAnim->mRotationKeys[k].mTime;
					Quaternion orientation = Quaternion(
						nodeAnim->mRotationKeys[k].mValue.w,
						nodeAnim->mRotationKeys[k].mValue.x,
						nodeAnim->mRotationKeys[k].mValue.y,
						nodeAnim->mRotationKeys[k].mValue.z);

					Transform transform;
					transform.orientation = orientation;
					NodeAnimation::Keyframe kf = { timepoint , transform };
					keyframes.push_back(kf);
				}
			}
			else if (node.name.find("Scaling") != std::string::npos)
			{
				node.type = NodeAnimation::Type::SCALE;
				keyframes.reserve(nodeAnim->mNumScalingKeys);
				for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; k++)
				{
					double timepoint = nodeAnim->mScalingKeys[k].mTime;
					float3 scale = float3(
						nodeAnim->mScalingKeys[k].mValue.x,
						nodeAnim->mScalingKeys[k].mValue.y,
						nodeAnim->mScalingKeys[k].mValue.z);

					Transform transform;
					transform.scale = scale;
					NodeAnimation::Keyframe kf = { timepoint , transform };
					keyframes.push_back(kf);
				}
			}
			else
			{
				continue;
			}
			animations.push_back(animation);
		}
	}
	return animations;
}

void ModelLoader::FillOutMaterialCreateInfo(aiMaterial* aiMaterial, Material::CreateInfo& materialCreateInfo)
{
	UniformBufferStructures::PBRConstants& pbrConstants = materialCreateInfo.pbrConstants;
	std::map<Material::TextureType, Ref<Texture>>& pbrTextures = materialCreateInfo.pbrTextures;

	pbrConstants = UniformBufferStructures::DefaultPBRConstants();

	bool useColourMap;
	bool useEmissiveMap;
	bool useMetallicMap;
	bool useRoughnessMap;
	bool useAOMap;

	aiString baseColourFilepath;
	aiString normalFilepath;
	aiString emissiveFilepath;
	aiString metallicFilepath;
	aiString roughnessFilepath;
	aiString aoFilepath;

	aiColor3D baseColour;
	aiColor3D emissiveColour;
	float emissiveIntensity = 0.0f;
	float metallic = 0.0f;
	float roughness = 0.0f;
	float ao = 0.0f;
	aiColor3D specularColour;

	aiShadingMode mode;
	aiMaterial->Get(AI_MATKEY_SHADING_MODEL, mode);

	aiMaterial->Get(AI_MATKEY_USE_COLOR_MAP, useColourMap);
	aiMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &baseColourFilepath);
	aiMaterial->Get(AI_MATKEY_BASE_COLOR, baseColour);

	aiMaterial->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &normalFilepath);

	aiMaterial->Get(AI_MATKEY_USE_EMISSIVE_MAP, useEmissiveMap);
	aiMaterial->GetTexture(aiTextureType_EMISSION_COLOR, 0, &emissiveFilepath);
	aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColour);
	aiMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveIntensity);
	
	aiMaterial->Get(AI_MATKEY_USE_METALLIC_MAP, useMetallicMap);
	aiMaterial->GetTexture(aiTextureType_METALNESS, 0, &metallicFilepath);
	aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic);

	aiMaterial->Get(AI_MATKEY_USE_ROUGHNESS_MAP, useRoughnessMap);
	aiMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessFilepath);
	aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

	aiMaterial->Get(AI_MATKEY_USE_AO_MAP, useAOMap);
	aiMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aoFilepath);

	aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColour);

	auto LoadTexture = [](std::map<Material::TextureType, Ref<Texture>>& textures, const Material::TextureType& type, aiString _filepath)
	{
		std::string filepath = std::string(_filepath.C_Str());

		if (std::filesystem::exists(filepath))
		{
			bool linear = Material::IsTextureTypeLinear(type);
			graphics::Texture::CreateInfo textureCI;
			textureCI.debugName = filepath;
			textureCI.device = m_Device;
			textureCI.dataType = graphics::Texture::DataType::FILE;
			textureCI.file.filepaths.push_back(filepath);
			textureCI.mipLevels = graphics::Texture::MaxMipLevel;
			textureCI.arrayLayers = 1;
			textureCI.type = miru::crossplatform::Image::Type::TYPE_2D;
			textureCI.format = linear ? miru::crossplatform::Image::Format::R32G32B32A32_SFLOAT : miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
			textureCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
			textureCI.usage = miru::crossplatform::Image::UsageBit(0);
			textureCI.generateMipMaps = true;
			textureCI.gammaSpace = linear ? graphics::GammaSpace::LINEAR : graphics::GammaSpace::SRGB;
			textures[type] = CreateRef<Texture>(&textureCI);
		}

	};
	LoadTexture(pbrTextures, Material::TextureType::ALBEDO, baseColourFilepath);
	LoadTexture(pbrTextures, Material::TextureType::NORMAL, normalFilepath);
	LoadTexture(pbrTextures, Material::TextureType::EMISSIVE, emissiveFilepath);
	LoadTexture(pbrTextures, Material::TextureType::METALLIC, metallicFilepath);
	LoadTexture(pbrTextures, Material::TextureType::ROUGHNESS, roughnessFilepath);
	LoadTexture(pbrTextures, Material::TextureType::AMBIENT_OCCLUSION, aoFilepath);

	auto aiColor3DToFloat4 = [](const aiColor3D value) -> float4
	{
		return float4(value.r, value.g, value.b, 1.0);
	};
	pbrConstants.fresnel = aiColor3DToFloat4(specularColour);
	pbrConstants.albedo = aiColor3DToFloat4(baseColour);
	pbrConstants.metallic = metallic;
	pbrConstants.roughness = roughness;
	pbrConstants.ambientOcclusion = 1.0f;
	pbrConstants.emissive = aiColor3DToFloat4(emissiveColour) * emissiveIntensity;
}
