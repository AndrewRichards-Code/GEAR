#include "material.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

Material::Material(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseUB();
}

Material::~Material()
{
}

void Material::AddTexture(gear::Ref<Texture> texture, TextureType type)
{
	m_Textures.insert(std::pair<gear::Ref<Texture>, TextureType>(texture, type));
}

void Material::ChangeTextureType(gear::Ref<Texture> texture, TextureType type)
{
	auto tex = m_Textures.find(texture);
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

void Material::AddProperties(const Properties& properties)
{
	m_Properties = properties;
}

void Material::SetPBRParameters(const mars::Vec4& fersnel, const mars::Vec4& albedo, float metallic, float roughness, float ambientOcclusion)
{
	m_PBRInfoUBO.fersnel = fersnel;
	m_PBRInfoUBO.albedo = albedo;
	m_PBRInfoUBO.metallic = metallic;
	m_PBRInfoUBO.roughness = roughness;
	m_PBRInfoUBO.ambientOcclusion = ambientOcclusion;
	m_PBRInfoUBO.pad = 0;

	m_UB->SubmitData((const void*)&m_PBRInfoUBO, sizeof(PBRInfoUBO));
}

void Material::InitialiseUB()
{
	m_UB = gear::CreateRef<UniformBuffer>(m_CI.device, sizeof(PBRInfoUBO), 4);

	const float zero[sizeof(PBRInfoUBO)] = { 0 };
	m_UB->SubmitData(zero, sizeof(PBRInfoUBO));
}