#include "gear_core_common.h"
#include "Material.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

std::map<std::string, Ref<Material>> Material::s_LoadedMaterials;
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
}

void Material::Update()
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
		m_CI.pbrTextures[TextureType::EMISSIVE] = s_BlackTexture;

	m_UB->fresnel			= m_CI.pbrConstants.fresnel;
	m_UB->albedo			= m_CI.pbrConstants.albedo;
	m_UB->metallic			= m_CI.pbrConstants.metallic;
	m_UB->roughness			= m_CI.pbrConstants.roughness;
	m_UB->ambientOcclusion	= m_CI.pbrConstants.ambientOcclusion;
	m_UB->pad				= 0.0f;
	m_UB->emissive			= m_CI.pbrConstants.emissive;
	m_UB->SubmitData();
}

void Material::AddProperties(const Properties& properties)
{
	m_Properties = properties;

	m_CI.pbrConstants.fresnel = m_Properties.colourSpecular;
	m_CI.pbrConstants.albedo = m_Properties.colourDiffuse;
	m_CI.pbrConstants.metallic;
	m_CI.pbrConstants.roughness = 1.0f - (m_Properties.shininess / 32.0f);
	m_CI.pbrConstants.ambientOcclusion;
	m_CI.pbrConstants.emissive = m_Properties.colourEmissive;
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
	texCI.dataType = Texture::DataType::DATA;
	texCI.data.data = dataWhite;
	texCI.data.size = 4;
	texCI.data.width = 1;
	texCI.data.height = 1;
	texCI.data.depth = 1;
	texCI.mipLevels = 1;
	texCI.arrayLayers = 1;
	texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
	texCI.format = miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
	texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	texCI.usage = miru::crossplatform::Image::UsageBit(0);
	texCI.generateMipMaps = false;
	s_WhiteTexture = CreateRef<Texture>(&texCI);

	uint8_t dataBlue[4] = { 0x7F, 0x7F, 0xFF, 0xFF };
	texCI.debugName = "GEAR_CORE_Material: Blank Blue-Normal Texture";
	texCI.data.data = dataBlue;
	s_BlueNormalTexture = CreateRef<Texture>(&texCI);

	uint8_t dataBlack[4] = { 0x00, 0x00, 0x00, 0xFF };
	texCI.debugName = "GEAR_CORE_Material: Blank Black Texture";
	texCI.data.data = dataBlack;
	s_BlackTexture = CreateRef<Texture>(&texCI);
}
