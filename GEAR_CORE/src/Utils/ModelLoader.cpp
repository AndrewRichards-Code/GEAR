#pragma once
#include "gear_core_common.h"
#include "ModelLoader.h"
#include "Objects/Material.h"

using namespace gear;

ModelLoader::ModelData ModelLoader::modelData;
void* ModelLoader::m_Device = nullptr;

ModelLoader::ModelData ModelLoader::LoadModelData(const std::string& filepath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath, aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		GEAR_ASSERT(core::Log::Level::WARN, core::Log::ErrorCode::UTILS | core::Log::ErrorCode::INIT_FAILED, "Assimp error: %s.", importer.GetErrorString());
		return ModelData(0);
	}
	modelData.clear();
	ProcessNode(scene->mRootNode, scene);
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
	meshData.name = std::string(node->mParent->mName.C_Str()) + ": " + std::string(mesh->mName.C_Str());

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

	//Materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	aiString materialName;
	material->Get(AI_MATKEY_NAME, materialName);

	/*gear::Ref<objects::Material> mat = objects::Material::FindMaterial(materialName.C_Str());
	if (mat != nullptr)
	{
		meshData.pMaterial = mat;
	}*/

	std::map<objects::Material::TextureType, gear::Ref<graphics::Texture>> textures;
	for (unsigned int i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
	{
		std::vector<std::string> filepaths = GetMaterialFilePath(material, (aiTextureType)i);
		if (!filepaths.empty())
		{
			objects::Material::TextureType type;
			for (auto& filepath : filepaths)
			{
				switch(i)
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

				graphics::Texture::CreateInfo texCI;
				texCI.device;
				texCI.filepaths = { filepath };
				texCI.data = nullptr;
				texCI.size = 0;
				texCI.width = 0;
				texCI.height = 0 ;
				texCI.depth = 0;
				texCI.mipLevels = 1;
				texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
				texCI.format = miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
				texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
				texCI.usage = miru::crossplatform::Image::UsageBit(0);
				texCI.generateMipMaps = false;
				textures[type] = CreateRef<graphics::Texture>(&texCI);
			}
		}
	}
	
	objects::Material::CreateInfo materialCI;
	materialCI.debugName = materialName.C_Str();
	materialCI.device = m_Device;
	materialCI.pbrTextures = textures;
	meshData.pMaterial = gear::CreateRef<objects::Material>(&materialCI);
	AddMaterialProperties(material, meshData.pMaterial);
	
	//objects::Material::AddMaterial(materialName.C_Str(), meshData.pMaterial);

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
		for(std::vector<std::string>::iterator it; it < result.end(); it++)
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
}
