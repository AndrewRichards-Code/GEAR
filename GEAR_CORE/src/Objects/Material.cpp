#include "gear_core_common.h"
#include "core/Databuffer.h"
#include "Graphics/Texture.h"
#include "Objects/Material.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

Ref<Texture> Material::s_WhiteTexture;
Ref<Texture> Material::s_BlueNormalTexture;
Ref<Texture> Material::s_BlackTexture;

Material::Material(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseUB();
	CreateDefaultColourTextures();
	Update();
}

Material::~Material()
{
	s_WhiteTexture = nullptr;
	s_BlueNormalTexture = nullptr;
	s_BlackTexture = nullptr;
}

void Material::Update()
{
	if (CreateInfoHasChanged(&m_CI))
	{
		SetDefaultPBRTextures();

		m_UB->fresnel = m_CI.pbrConstants.fresnel;
		m_UB->albedo = m_CI.pbrConstants.albedo;
		m_UB->metallic = m_CI.pbrConstants.metallic;
		m_UB->roughness = m_CI.pbrConstants.roughness;
		m_UB->ambientOcclusion = m_CI.pbrConstants.ambientOcclusion;
		m_UB->pad = 0.0f;
		m_UB->emissive = m_CI.pbrConstants.emissive;
		m_UB->SubmitData();
	}
}

bool Material::CreateInfoHasChanged(const ObjectComponentInterface::CreateInfo* pCreateInfo)
{
	const CreateInfo& CI = *reinterpret_cast<const CreateInfo*>(pCreateInfo);
	uint64_t newHash = 0;
	for (const auto& texture : CI.pbrTextures)
	{
		newHash ^= core::GetHash(texture.first);
		newHash ^= core::GetHash((uint64_t)texture.second.get());
	}
	newHash ^= core::GetHash(CI.pbrConstants.fresnel.r);
	newHash ^= core::GetHash(CI.pbrConstants.fresnel.g);
	newHash ^= core::GetHash(CI.pbrConstants.fresnel.b);
	newHash ^= core::GetHash(CI.pbrConstants.fresnel.a);
	newHash ^= core::GetHash(CI.pbrConstants.albedo.r);
	newHash ^= core::GetHash(CI.pbrConstants.albedo.g);
	newHash ^= core::GetHash(CI.pbrConstants.albedo.b);
	newHash ^= core::GetHash(CI.pbrConstants.albedo.a);
	newHash ^= core::GetHash(CI.pbrConstants.metallic);
	newHash ^= core::GetHash(CI.pbrConstants.roughness);
	newHash ^= core::GetHash(CI.pbrConstants.ambientOcclusion);
	newHash ^= core::GetHash(CI.pbrConstants.emissive.r);
	newHash ^= core::GetHash(CI.pbrConstants.emissive.g);
	newHash ^= core::GetHash(CI.pbrConstants.emissive.b);
	newHash ^= core::GetHash(CI.pbrConstants.emissive.a);
	return CompareCreateInfoHash(newHash);
}

void Material::SetDefaultPBRTextures()
{
	if (!m_CI.pbrTextures[TextureType::NORMAL])
		m_CI.pbrTextures[TextureType::NORMAL] = s_BlueNormalTexture;
	if (!m_CI.pbrTextures[TextureType::ALBEDO])
		m_CI.pbrTextures[TextureType::ALBEDO] = s_WhiteTexture;
	if (!m_CI.pbrTextures[TextureType::METALLIC])
		m_CI.pbrTextures[TextureType::METALLIC] = s_WhiteTexture;
	if (!m_CI.pbrTextures[TextureType::ROUGHNESS])
		m_CI.pbrTextures[TextureType::ROUGHNESS] = s_WhiteTexture;
	if (!m_CI.pbrTextures[TextureType::AMBIENT_OCCLUSION])
		m_CI.pbrTextures[TextureType::AMBIENT_OCCLUSION] = s_WhiteTexture;
	if (!m_CI.pbrTextures[TextureType::EMISSIVE])
		m_CI.pbrTextures[TextureType::EMISSIVE] = s_WhiteTexture;
}

void Material::InitialiseUB()
{
	float zero[sizeof(PBRConstantsUB)] = { 0 };
	Uniformbuffer<PBRConstantsUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Material_PBRConstants: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;

	m_UB = CreateRef<Uniformbuffer<PBRConstantsUB>>(&ubCI);
}

void Material::CreateDefaultColourTextures()
{
	if (s_WhiteTexture && s_BlueNormalTexture && s_BlackTexture)
		return;
	
	uint8_t dataWhite[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	Texture::CreateInfo texCI;
	texCI.debugName = "GEAR_CORE_Material: Blank White Texture";
	texCI.device = m_CI.device;
	texCI.imageData = core::DataBuffer(dataWhite, 4);
	texCI.width = 1;
	texCI.height = 1;
	texCI.depth = 1;
	texCI.mipLevels = 1;
	texCI.arrayLayers = 1;
	texCI.type = miru::base::Image::Type::TYPE_2D;
	texCI.format = miru::base::Image::Format::R8G8B8A8_UNORM;
	texCI.samples = miru::base::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	texCI.usage = miru::base::Image::UsageBit(0);
	texCI.generateMipMaps = false;
	texCI.gammaSpace = GammaSpace::LINEAR;
	s_WhiteTexture = CreateRef<Texture>(&texCI);

	uint8_t dataBlue[4] = { 0x7F, 0x7F, 0xFF, 0xFF };
	texCI.debugName = "GEAR_CORE_Material: Blank Blue-Normal Texture";
	texCI.imageData = core::DataBuffer(dataBlue, 4);
	s_BlueNormalTexture = CreateRef<Texture>(&texCI);

	uint8_t dataBlack[4] = { 0x00, 0x00, 0x00, 0xFF };
	texCI.debugName = "GEAR_CORE_Material: Blank Black Texture";
	texCI.imageData = core::DataBuffer(dataBlack, 4);
	s_BlackTexture = CreateRef<Texture>(&texCI);
}