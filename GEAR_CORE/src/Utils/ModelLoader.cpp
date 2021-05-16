#pragma once
#include "gear_core_common.h"
#include "ModelLoader.h"
#include "Objects/Material.h"
#include "Objects/Transform.h"
#include "ARC/src/FileSystemHelpers.h"

using namespace gear;

ModelLoader::ModelData ModelLoader::modelData;
std::vector<ModelLoader::Animation> ModelLoader::animationData;
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
		return ModelData(0);
	}
	modelData.clear();
	ProcessNode(scene->mRootNode, scene);
	ProcessAnimations(scene);
	return std::move(modelData);
}

void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		modelData.push_back(ProcessMesh(mesh, node, scene));
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

ModelLoader::MeshData ModelLoader::ProcessMesh(aiMesh* mesh, aiNode* node, const aiScene* scene)
{
	MeshData meshData;
	//meshData.name = std::string(node->mParent->mName.C_Str()) + ": " + std::string(mesh->mName.C_Str());

	//Vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex = {};
		if (mesh->HasPositions())
		{
			vertex.position= mars::Vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
		}
		if (mesh->HasTextureCoords(0))
		{
			vertex.texCoord = mars::Vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		if (mesh->HasNormals())
		{
			vertex.normal = mars::Vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0.0f);
		}
		if (mesh->HasTangentsAndBitangents())
		{
			vertex.tangent = mars::Vec4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 0.0f);
			vertex.binormal = mars::Vec4(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z, 0.0f);
		}
		if (mesh->HasVertexColors(0))
		{
			vertex.colour = mars::Vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
		}
		meshData.vertices.push_back(vertex);
	}

	//Indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			meshData.indices.push_back(face.mIndices[j]);
	}

	//Bones
	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		mars::Mat4 transform;
		aiMatrix4x4 aiTransform = mesh->mBones[i]->mOffsetMatrix;
		memcpy_s((void* const)transform.GetData(), transform.GetSize(), &aiTransform.a1, sizeof(aiMatrix4x4));

		std::vector<std::pair<uint32_t, float>> vertexIDsAndWeights;
		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
		{
			const aiVertexWeight& weight = mesh->mBones[i]->mWeights[j];
			vertexIDsAndWeights.push_back({ weight.mVertexId, weight.mWeight });
		}
		meshData.bones.push_back({});
		meshData.bones.back().transform = std::move(transform);
		meshData.bones.back().vertexIDsAndWeights = std::move(vertexIDsAndWeights);
	}

	//Materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	aiString materialName;
	material->Get(AI_MATKEY_NAME, materialName);

	Ref<objects::Material> mat = objects::Material::FindMaterial(materialName.C_Str());
	if (mat != nullptr)
	{
		meshData.pMaterial = mat;
	}
	else
	{
		std::map<objects::Material::TextureType, Ref<graphics::Texture>> textures;
		for (unsigned int i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
		{
			std::vector<std::string> filepaths = GetMaterialFilePath(material, (aiTextureType)i);
			if (!filepaths.empty())
			{
				objects::Material::TextureType type;
				for (auto& filepath : filepaths)
				{
					switch (i)
					{
					case aiTextureType::aiTextureType_BASE_COLOR:
						type = objects::Material::TextureType::ALBEDO; break;
					case aiTextureType::aiTextureType_NORMAL_CAMERA:
						type = objects::Material::TextureType::NORMAL; break;
					case aiTextureType::aiTextureType_EMISSION_COLOR:
						type = objects::Material::TextureType::EMISSIVE; break;
					case aiTextureType::aiTextureType_METALNESS:
						type = objects::Material::TextureType::METALLIC; break;
					case aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS:
						type = objects::Material::TextureType::ROUGHNESS; break;
					case aiTextureType::aiTextureType_AMBIENT_OCCLUSION:
						type = objects::Material::TextureType::AMBIENT_OCCLUSION; break;
					case aiTextureType::aiTextureType_NORMALS:
						type = objects::Material::TextureType::NORMAL; break;
					default:
						type = objects::Material::TextureType::UNKNOWN; break;
					}

					if (arc::FileExist(filepath))
					{
						graphics::Texture::CreateInfo texCI;
						texCI.device = m_Device;
						texCI.dataType = graphics::Texture::DataType::FILE;
						texCI.file.filepaths = &filepath;
						texCI.file.count = 1;
						texCI.mipLevels = 1;
						texCI.arrayLayers = 1;
						texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
						texCI.format = miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
						texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
						texCI.usage = miru::crossplatform::Image::UsageBit(0);
						texCI.generateMipMaps = false;
						textures[type] = CreateRef<graphics::Texture>(&texCI);
					}
				}
			}
		}

		objects::Material::CreateInfo materialCI;
		materialCI.debugName = materialName.C_Str();
		materialCI.device = m_Device;
		materialCI.pbrTextures = textures;
		meshData.pMaterial = CreateRef<objects::Material>(&materialCI);
		AddMaterialProperties(material, meshData.pMaterial);

		objects::Material::AddMaterial(materialName.C_Str(), meshData.pMaterial);
	}

	return meshData;
}

std::vector<std::string> ModelLoader::GetMaterialFilePath(aiMaterial* material, aiTextureType type)
{
	std::vector<std::string> result;
	for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
	{
		aiString str;
		material->GetTexture(type, i, &str);
		std::string filepath = str.C_Str();

		bool skip = false;
		for(std::vector<std::string>::iterator it = result.begin(); it < result.end(); it++)
		{
			if (*it == filepath)
			{
				skip = true;
				break;
			}
		}
		if(!skip)
			result.push_back(filepath);
	}
	return result;
}

void ModelLoader::AddMaterialProperties(aiMaterial* aiMaterial, Ref<objects::Material> material)
{
	aiString name;
	int twoSided;
	int shadingModel;
	int wireframe;
	int blendFunc;
	float opacity;
	float shininess;
	float reflectivity;
	float shininessStrength;
	float refractiveIndex;
	aiColor3D colourDiffuse;
	aiColor3D colourAmbient;
	aiColor3D colourSpecular;
	aiColor3D colourEmissive;
	aiColor3D colourTransparent;
	aiColor3D colourReflective;

	aiMaterial->Get(AI_MATKEY_NAME, name);
	aiMaterial->Get(AI_MATKEY_TWOSIDED, twoSided);
	aiMaterial->Get(AI_MATKEY_SHADING_MODEL, shadingModel);
	aiMaterial->Get(AI_MATKEY_ENABLE_WIREFRAME, wireframe);
	aiMaterial->Get(AI_MATKEY_BLEND_FUNC, blendFunc);
	aiMaterial->Get(AI_MATKEY_OPACITY, opacity);
	//aiMmaterial->Get(AI_MATKEY_BUMPSCALING, colour_diffuse);
	aiMaterial->Get(AI_MATKEY_SHININESS, shininess);
	aiMaterial->Get(AI_MATKEY_REFLECTIVITY, reflectivity);
	aiMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength);
	aiMaterial->Get(AI_MATKEY_REFRACTI, refractiveIndex);
	aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, colourDiffuse);
	aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, colourAmbient);
	aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, colourSpecular);
	aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, colourEmissive);
	aiMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, colourTransparent);
	aiMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, colourReflective);
	//aiMaterial->Get(AI_MATKEY_GLOBAL_BACKGROUND_IMAGE, colour_diffuse);

	material->AddProperties({
		name.C_Str(),
		twoSided,
		shadingModel,
		wireframe,
		blendFunc,
		opacity,
		shininess,
		reflectivity,
		shininessStrength,
		refractiveIndex,
		mars::Vec4(colourDiffuse.r, colourDiffuse.g, colourDiffuse.b, 1),
		mars::Vec4(colourAmbient.r, colourAmbient.g, colourAmbient.b, 1),
		mars::Vec4(colourSpecular.r, colourSpecular.g, colourSpecular.b, 1),
		mars::Vec4(colourEmissive.r, colourEmissive.g, colourEmissive.b, 1),
		mars::Vec4(colourTransparent.r, colourTransparent.g, colourTransparent.b, 1),
		mars::Vec4(colourReflective.r, colourReflective.g, colourReflective.b, 1)
		});
	material->Update();
}

void ModelLoader::ProcessAnimations(const aiScene* scene)
{
	for (unsigned int i = 0; i < scene->mNumAnimations; i++)
	{
		Animation animation;
		aiAnimation*& _animation = scene->mAnimations[i];

		animation.duration = _animation->mDuration;
		animation.framesPerSecond = static_cast<uint32_t>(_animation->mTicksPerSecond);
		
		for (unsigned int j = 0; j < _animation->mNumChannels; j++)
		{
			aiNodeAnim*& nodeAnim = _animation->mChannels[j];

			animation.nodeAnimations.push_back({});
			NodeAnimation& node = animation.nodeAnimations.back();
			node.name = std::string(nodeAnim->mNodeName.C_Str());
			NodeAnimation::Keyframes& keyframes = node.keyframes;

			if (node.name.find("Translation") != std::string::npos) 
			{
				for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++)
				{
					double timepoint = nodeAnim->mPositionKeys[k].mTime;
					mars::Vec3 translation = mars::Vec3(
						nodeAnim->mPositionKeys[k].mValue.x,
						nodeAnim->mPositionKeys[k].mValue.y,
						nodeAnim->mPositionKeys[k].mValue.z);

					objects::Transform transform;
					transform.translation = translation;
					NodeAnimation::Keyframe kf = { timepoint , transform };
					keyframes.push_back(kf);
				}
			}
			else if (node.name.find("Rotation") != std::string::npos) 
			{
				for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; k++)
				{
					double timepoint = nodeAnim->mRotationKeys[k].mTime;
					mars::Quat orientation = mars::Quat(
						nodeAnim->mRotationKeys[k].mValue.w,
						nodeAnim->mRotationKeys[k].mValue.x,
						nodeAnim->mRotationKeys[k].mValue.y,
						nodeAnim->mRotationKeys[k].mValue.z);

					objects::Transform transform;
					transform.orientation = orientation;
					NodeAnimation::Keyframe kf = { timepoint , transform };
					keyframes.push_back(kf);
				}
			}
			else if (node.name.find("Scale") != std::string::npos) 
			{
				for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; k++)
				{
					double timepoint = nodeAnim->mScalingKeys[k].mTime;
					mars::Vec3 scale = mars::Vec3(
						nodeAnim->mScalingKeys[k].mValue.x,
						nodeAnim->mScalingKeys[k].mValue.y,
						nodeAnim->mScalingKeys[k].mValue.z);

					objects::Transform transform;
					transform.scale = scale;
					NodeAnimation::Keyframe kf = { timepoint , transform };
					keyframes.push_back(kf);
				}
			}
			else
			{
				continue;
			}

		}
		animationData.push_back(std::move(animation));
	}
}
