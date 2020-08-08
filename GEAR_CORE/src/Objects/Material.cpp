#include "gear_core_common.h"
#include "Material.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

std::map<std::string, Ref<Material>> Material::s_LoadedMaterials;
//gear::Ref<Texture> Material::s_WhiteTexture;
//gear::Ref<Texture> Material::s_BlueTexture;
//gear::Ref<Texture> Material::s_BlackTexture;

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
		m_CI.pbrTextures[TextureType::NORMAL] = s_BlueTexture;
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

	m_UB->fersnel			= m_CI.pbrConstants.fersnel;
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
}

void Material::InitialiseUB()
{
	float zero[sizeof(PBRConstants)] = { 0 };
	Uniformbuffer<PBRConstants>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Material: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;

	m_UB = gear::CreateRef<Uniformbuffer<PBRConstants>>(&ubCI);
}

void Material::CreateDefaultColourTextures()
{
	if (s_WhiteTexture && s_BlueTexture && s_BlackTexture)
		return;
	
	uint8_t dataWhite[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	Texture::CreateInfo texCI;
	texCI.debugName = "GEAR_CORE_Material: Blank Texture";
	texCI.device = m_CI.device;
	texCI.data = dataWhite;
	texCI.size = 4;
	texCI.width = 1;
	texCI.height = 1;
	texCI.depth = 1;
	texCI.format = miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
	texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
	texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	s_WhiteTexture = gear::CreateRef<Texture>(&texCI);

	uint8_t dataBlue[4] = { 0x00, 0x00, 0xFF, 0xFF };
	texCI.data = dataBlue;
	s_BlueTexture = gear::CreateRef<Texture>(&texCI);

	uint8_t dataBlack[4] = { 0x00, 0x00, 0x0, 0xFF };
	texCI.data = dataBlack;
	s_BlackTexture = gear::CreateRef<Texture>(&texCI);
}
