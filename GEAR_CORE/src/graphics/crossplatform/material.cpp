#include "material.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace CROSSPLATFORM;
using namespace ARM;

bool Material::s_InitialiseUBO = false;

Material::Material(const OPENGL::Shader& shader)
	:m_Shader(shader)
{
	InitialiseUBO();
}

Material::~Material()
{
	SetAllToZero();
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
			std::cout << "ERROR: GEAR::GRAPHICS::CROSSPLATFORM::Material: Can't change TextureType. The new type is the same as the previous." << std::endl;
			return;
		}
		else
			tex->second = type;
	}
	else
		std::cout << "ERROR: GEAR::GRAPHICS::CROSSPLATFORM::Material: Can't change TextureType. The texture was not found." << std::endl;

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
	const ARM::Vec4& colourDiffuse,
	const ARM::Vec4& colourAmbient,
	const ARM::Vec4& colourSpecular,
	const ARM::Vec4& colourEmissive,
	const ARM::Vec4& colourTransparent,
	const ARM::Vec4& colourReflective)
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

void Material::SetPBRParameters(const ARM::Vec4& fersnel, const ARM::Vec4& albedo, float metallic, float roughness, float ambientOcclusion)
{
	m_PBRInfoUBO.m_Fersnel = fersnel;
	m_PBRInfoUBO.m_Albedo = albedo;
	m_PBRInfoUBO.m_Metallic = metallic;
	m_PBRInfoUBO.m_Roughness = roughness;
	m_PBRInfoUBO.m_AmbientOcclusion = ambientOcclusion;
	m_PBRInfoUBO.m_Pad = 0;

	OPENGL::BufferManager::UpdateUBO(4, (const float*)&m_PBRInfoUBO, sizeof(PBRInfoUBO), 0);
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