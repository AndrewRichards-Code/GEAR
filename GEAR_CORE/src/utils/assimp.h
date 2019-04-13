#pragma once
#ifdef _M_X64
#include "gear_common.h"
#include "maths/ARMLib.h"
#include "graphics/crossplatform/material.h"

namespace GEAR {

class AssimpLoader
{
public:
	struct Vertex
	{
		ARM::Vec3 m_Vertex;
		ARM::Vec2 m_TexCoord;
		float m_TexId;
		ARM::Vec3 m_Normal;
		ARM::Vec4 m_Colour;
	};
	struct Mesh
	{
		std::vector<Vertex>	m_Vertices;
		std::vector<unsigned int> m_Indices;
		GRAPHICS::CROSSPLATFORM::Material m_Material = GRAPHICS::CROSSPLATFORM::Material(GRAPHICS::OPENGL::Shader("res/shaders/GLSL/pbr.vert", "res/shaders/GLSL/pbr.frag"));
	};

	static std::vector<Mesh> result;
	static std::vector<Mesh> LoadModel(const std::string& filepath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR: GEAR::AssimpLoader::LoadModel: " << importer.GetErrorString() << std::endl;
			return std::vector<Mesh>(0);
		}
		result.clear();
		ProcessNode(scene->mRootNode, scene);
		return result;
	}

private:
	static void ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			result.push_back(ProcessMesh(mesh, scene));
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	static Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		Mesh result;
		//Vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex = {};
			if(mesh->HasPositions())
				vertex.m_Vertex = ARM::Vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			if(mesh->HasTextureCoords(0))
				vertex.m_TexCoord = ARM::Vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			if(mesh->HasNormals())
				vertex.m_Normal = ARM::Vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			if(mesh->HasVertexColors(0))
				vertex.m_Colour = ARM::Vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
			
			vertex.m_TexId = 0.0f;
			result.m_Vertices.push_back(vertex);
		}

		//Indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				result.m_Indices.push_back(face.mIndices[j]);
		}

		//Materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		for (unsigned int i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
		{
			std::vector<std::string> filepaths = GetMaterialFilePath(material, (aiTextureType)i);
			if (!filepaths.empty())
			{
				GRAPHICS::CROSSPLATFORM::Material::TextureType type;
				for (auto& filepath : filepaths)
				{
					switch(i)
					{
					case aiTextureType::aiTextureType_NONE:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_UNKNOWN; break;
					case aiTextureType::aiTextureType_DIFFUSE:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_DIFFUSE; break;
					case aiTextureType::aiTextureType_SPECULAR:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_SPECULAR; break;
					case aiTextureType::aiTextureType_AMBIENT:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_AMBIENT; break;
					case aiTextureType::aiTextureType_EMISSIVE:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_EMISSIVE; break;
					case aiTextureType::aiTextureType_HEIGHT:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_HEIGHT; break;
					case aiTextureType::aiTextureType_NORMALS:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_NORMAL; break;
					case aiTextureType::aiTextureType_SHININESS:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_SMOOTHNESS; break;
					case aiTextureType::aiTextureType_OPACITY:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_OPACITY; break;
					case aiTextureType::aiTextureType_LIGHTMAP:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_AMBIENT_OCCLUSION; break;
					case aiTextureType::aiTextureType_REFLECTION:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_REFLECTION; break;
					case aiTextureType::aiTextureType_UNKNOWN:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_UNKNOWN; break;
					default:
						type = GRAPHICS::CROSSPLATFORM::Material::TextureType::GEAR_TEXTURE_UNKNOWN; break;
					}
					result.m_Material.AddTexture(GRAPHICS::OPENGL::Texture(filepath), type);
				}
			}
		}
		AddMaterialProperties(material, result.m_Material);

		return result;
	}

	static std::vector<std::string> GetMaterialFilePath(aiMaterial* material, aiTextureType type)
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

	static void AddMaterialProperties(aiMaterial* aiMaterial, GRAPHICS::CROSSPLATFORM::Material& material)
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

		material.AddProperties(
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
			ARM::Vec4(colourDiffuse.r, colourDiffuse.g, colourDiffuse.b, 1),
			ARM::Vec4(colourAmbient.r, colourAmbient.g, colourAmbient.b, 1),
			ARM::Vec4(colourSpecular.r, colourSpecular.g, colourSpecular.b, 1),
			ARM::Vec4(colourEmissive.r, colourEmissive.g, colourEmissive.b, 1),
			ARM::Vec4(colourTransparent.r, colourTransparent.g, colourTransparent.b, 1),
			ARM::Vec4(colourReflective.r, colourReflective.g, colourReflective.b, 1)
		);
	}
};
std::vector<AssimpLoader::Mesh> AssimpLoader::result;
}
#endif