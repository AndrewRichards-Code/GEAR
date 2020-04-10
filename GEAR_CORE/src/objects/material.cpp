#include "material.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OBJECTS;
using namespace mars;

bool Material::s_InitialiseUBO = false;

Material::Material(const OPENGL::Shader& shader)
	:m_Shader(shader)
{
	InitialiseUBO();
}

Material::~Material()
{
	UnbindPBRTextures();
}

void Material::AddTexture(const OPENGL::Texture& texture, TextureType type)
{
	m_Textures.insert(std::pair<const OPENGL::Texture*, TextureType>(&texture, type));
}

void Material::ChangeTextureType(const OPENGL::Texture& texture, TextureType type)
{
	auto tex = m_Textures.find(&texture);
	if (tex != m_Textures.end())
	{
		if (tex->second == type)
		{
			std::cout << "ERROR: GEAR::GRAPHICS::OBJECTS::Material: Can't change TextureType. The new type is the same as the previous." << std::endl;
			return;
		}
		else
			tex->second = type;
	}
	else
		std::cout << "ERROR: GEAR::GRAPHICS::OBJECTS::Material: Can't change TextureType. The texture was not found." << std::endl;

}

void Material::AddProperties(
	std::string name,
	int twoSided,
	int shadingModel,
	int wireframe,
	int blendFunc,
	float opacity,
	float shininess,
	float reflectivity,
	float shininessStrength,
	float refractiveIndex,
	const mars::Vec4& colourDiffuse,
	const mars::Vec4& colourAmbient,
	const mars::Vec4& colourSpecular,
	const mars::Vec4& colourEmissive,
	const mars::Vec4& colourTransparent,
	const mars::Vec4& colourReflective)
{
	m_Properties.m_Name = name;
	m_Properties.m_TwoSided = twoSided;
	m_Properties.m_ShadingModel = shadingModel;
	m_Properties.m_Wireframe = wireframe;
	m_Properties.m_BlendFunc = blendFunc;
	m_Properties.m_Opacity = opacity;
	m_Properties.m_Shininess = shininess;
	m_Properties.m_Reflectivity = reflectivity;
	m_Properties.m_ShininessStrength = shininessStrength;
	m_Properties.m_RefractiveIndex = refractiveIndex;
	m_Properties.m_ColourDiffuse = colourDiffuse;
	m_Properties.m_ColourAmbient = colourAmbient;
	m_Properties.m_ColourSpecular = colourSpecular;
	m_Properties.m_ColourEmissive = colourEmissive;
	m_Properties.m_ColourTransparent = colourTransparent;
	m_Properties.m_ColourReflective = colourReflective;
}

void Material::SetPBRParameters(const mars::Vec4& fersnel, const mars::Vec4& albedo, float metallic, float roughness, float ambientOcclusion)
{
	m_PBRInfoUBO.m_Fersnel = fersnel;
	m_PBRInfoUBO.m_Albedo = albedo;
	m_PBRInfoUBO.m_Metallic = metallic;
	m_PBRInfoUBO.m_Roughness = roughness;
	m_PBRInfoUBO.m_AmbientOcclusion = ambientOcclusion;
	m_PBRInfoUBO.m_Pad = 0;

	OPENGL::BufferManager::UpdateUBO(4, (const float*)&m_PBRInfoUBO, sizeof(PBRInfoUBO), 0);
}

void Material::BindPBRTextures()
{
	int slot = 10;
	m_Shader.Enable();
	for (auto& texture : m_Textures)
	{
		std::string uniformName;
		switch (texture.second)
		{
		case TextureType::GEAR_TEXTURE_ALBEDO:
			uniformName = "u_TextureAlbedo"; break;
		case TextureType::GEAR_TEXTURE_METALLIC:
			uniformName = "u_TextureMetallic"; break;
		case TextureType::GEAR_TEXTURE_ROUGHNESS:
			uniformName = "u_TextureRoughness"; break;
		case TextureType::GEAR_TEXTURE_AMBIENT_OCCLUSION:
			uniformName = "u_TextureAO"; break;
		case TextureType::GEAR_TEXTURE_NORMAL:
			uniformName = "u_TextureNormal"; break;
		case TextureType::GEAR_TEXTURE_AMBIENT:
			uniformName = "u_TextureEnvironment"; break;
		}
		texture.first->Bind(slot);
		m_Shader.SetUniform<int>(uniformName, { slot });
		slot++;
	}
	m_Shader.Disable();
}

void Material::UnbindPBRTextures()
{
	for (auto& texture : m_Textures)
	{
		texture.first->Unbind();
	}
}

void Material::InitialiseUBO()
{
	if (s_InitialiseUBO == false)
	{
		OPENGL::BufferManager::AddUBO(sizeof(PBRInfoUBO), 4);
		s_InitialiseUBO = true;

		const float zero[sizeof(PBRInfoUBO)] = { 0 };
		OPENGL::BufferManager::UpdateUBO(4, zero, sizeof(PBRInfoUBO), 0);
	}
}

void Material::SetAllToZero()
{
	const float zero[sizeof(PBRInfoUBO)] = { 0 };
	OPENGL::BufferManager::UpdateUBO(4, zero, sizeof(PBRInfoUBO), 0);
}