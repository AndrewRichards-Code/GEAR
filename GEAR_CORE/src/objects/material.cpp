#include "gear_core_common.h"
#include "Material.h"

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

void Material::SetPBRParameters(const Vec4& fersnel, const Vec4& albedo, float metallic, float roughness, float ambientOcclusion)
{
	m_UB->fersnel = fersnel;
	m_UB->albedo = albedo;
	m_UB->metallic = metallic;
	m_UB->roughness = roughness;
	m_UB->ambientOcclusion = ambientOcclusion;
	m_UB->pad = 0;
	m_UB->SubmitData();
}

void Material::InitialiseUB()
{
	float zero[sizeof(PBRInfoUB)] = { 0 };
	Uniformbuffer<PBRInfoUB>::CreateInfo ubCI;
	ubCI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;

	m_UB = gear::CreateRef<Uniformbuffer<PBRInfoUB>>(&ubCI);

}