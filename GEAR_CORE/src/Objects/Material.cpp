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

	if (!m_CI.filepath.empty())
		LoadFromAssetFile(core::AssetFile(m_CI.filepath));

	Update();
}

Material::~Material()
{
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


void Material::AddProperties(const Properties& properties)
{
	m_Properties = properties;

	m_CI.pbrConstants.fresnel = m_Properties.colourSpecular;
	m_CI.pbrConstants.albedo = m_Properties.colourDiffuse;
	m_CI.pbrConstants.metallic = 1.0f;
	m_CI.pbrConstants.roughness = 1.0f - (m_Properties.shininess / 32.0f);
	m_CI.pbrConstants.ambientOcclusion = 1.0f;
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

void Material::LoadFromAssetFile(const core::AssetFile& inAssetFile)
{
	if (inAssetFile.m_AssetData.find("material") == inAssetFile.m_AssetData.end())
		return;

	const nlohmann::json& material_gaf = inAssetFile.m_AssetData["material"];
	m_CI.debugName = material_gaf["debugName"];

	const nlohmann::json& pbrConstants = material_gaf["pbrConstants"];
	m_CI.pbrConstants.fresnel.r = pbrConstants["fresnel"][0];
	m_CI.pbrConstants.fresnel.g = pbrConstants["fresnel"][1];
	m_CI.pbrConstants.fresnel.b = pbrConstants["fresnel"][2];
	m_CI.pbrConstants.fresnel.a = pbrConstants["fresnel"][3];
	m_CI.pbrConstants.albedo.r = pbrConstants["albedo"][0];
	m_CI.pbrConstants.albedo.g = pbrConstants["albedo"][1];
	m_CI.pbrConstants.albedo.b = pbrConstants["albedo"][2];
	m_CI.pbrConstants.albedo.a = pbrConstants["albedo"][3];
	m_CI.pbrConstants.metallic = pbrConstants["metallic"];
	m_CI.pbrConstants.roughness = pbrConstants["roughness"];
	m_CI.pbrConstants.ambientOcclusion = pbrConstants["ambientOcclusion"];
	m_CI.pbrConstants.emissive.r = pbrConstants["emissive"][0];
	m_CI.pbrConstants.emissive.g = pbrConstants["emissive"][1];
	m_CI.pbrConstants.emissive.b = pbrConstants["emissive"][2];
	m_CI.pbrConstants.emissive.a = pbrConstants["emissive"][3];

	m_CI.pbrTextures.clear();
	SetDefaultPBRTextures();

	const nlohmann::json& textures = material_gaf["textures"];
	Texture::CreateInfo texCI;
	for (const auto& texture : textures)
	{
		TextureType textureType = TextureType::UNKNOWN;
		Ref<Texture> textureRef = nullptr;

		std::string textureTypeStr = arc::ToUpper(std::string(texture["textureType"]));

		if(textureTypeStr.compare("UNKNOWN")==0)
			textureType = TextureType::UNKNOWN;
		else if(textureTypeStr.compare("NORMAL")==0)
			textureType = TextureType::NORMAL;
		else if(textureTypeStr.compare("ALBEDO")==0)
			textureType = TextureType::ALBEDO;
		else if(textureTypeStr.compare("METALLIC")==0)
			textureType = TextureType::METALLIC;
		else if(textureTypeStr.compare("ROUGHNESS")==0)
			textureType = TextureType::ROUGHNESS;
		else if(textureTypeStr.compare("AMBIENT_OCCLUSION")==0)
			textureType = TextureType::AMBIENT_OCCLUSION;
		else if(textureTypeStr.compare("EMISSIVE")==0)
			textureType = TextureType::EMISSIVE;
		else
			textureType = TextureType::UNKNOWN;

		if (textureType == TextureType::UNKNOWN)
			continue;
			
		bool linear = texture["linear"];
		texCI.debugName = texture["debugName"];
		texCI.device = m_CI.device;
		texCI.dataType = Texture::DataType::FILE;
		texCI.file.filepaths = { texture["filepath"] };
		texCI.mipLevels = graphics::Texture::MaxMipLevel;
		texCI.arrayLayers = 1;
		texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
		texCI.format = linear ? miru::crossplatform::Image::Format::R32G32B32A32_SFLOAT : miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
		texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		texCI.usage = miru::crossplatform::Image::UsageBit(0);
		texCI.generateMipMaps = true;
		texCI.gammaSpace = linear ? GammaSpace::LINEAR : GammaSpace::SRGB;
		m_CI.pbrTextures[textureType] = CreateRef<Texture>(&texCI);
	}

	m_CI.filepath = inAssetFile.m_CI.filepath;
}

void Material::SaveToAssetFile(core::AssetFile& outAssetFile)
{
	nlohmann::json& material_gaf = outAssetFile.m_AssetData["material"];
	material_gaf["debugName"] = m_CI.debugName;

	nlohmann::json& pbrConstants = material_gaf["pbrConstants"];
	pbrConstants["fresnel"] = { m_CI.pbrConstants.fresnel.r, m_CI.pbrConstants.fresnel.g, m_CI.pbrConstants.fresnel.b, m_CI.pbrConstants.fresnel.a };
	pbrConstants["albedo"] = { m_CI.pbrConstants.albedo.r, m_CI.pbrConstants.albedo.g, m_CI.pbrConstants.albedo.b, m_CI.pbrConstants.albedo.a };
	pbrConstants["metallic"] = m_CI.pbrConstants.metallic;
	pbrConstants["roughness"] = m_CI.pbrConstants.roughness;
	pbrConstants["ambientOcclusion"] = m_CI.pbrConstants.ambientOcclusion;
	pbrConstants["emissive"] = { m_CI.pbrConstants.emissive.r, m_CI.pbrConstants.emissive.g, m_CI.pbrConstants.emissive.b, m_CI.pbrConstants.emissive.a };

	nlohmann::json& textures = material_gaf["textures"];
	for (const auto& texture : m_CI.pbrTextures)
	{
		const TextureType& textureType = texture.first;
		const Ref<Texture>& textureRef = texture.second;

		std::string type = "";
		switch (texture.first)
		{
		default:
		case TextureType::UNKNOWN:
			break;
		case TextureType::NORMAL:
			type = "normal"; break;
		case TextureType::ALBEDO:
			type = "albedo"; break;
		case TextureType::METALLIC:
			type = "metallic"; break;
		case TextureType::ROUGHNESS:
			type = "roughness"; break;
		case TextureType::AMBIENT_OCCLUSION:
			type = "ambientOcclusion"; break;
		case TextureType::EMISSIVE:
			type = "emissive"; break;
		}
		if (type.empty())
			continue;

		const Texture::CreateInfo& textureCI = textureRef->GetCreateInfo();
		const std::vector<std::string>& filepaths = textureCI.file.filepaths;
		if (!filepaths.empty())
		{
			textures[type.c_str()]["textureType"] = type.c_str();
			textures[type.c_str()]["linear"] = textureCI.gammaSpace == GammaSpace::LINEAR ? true : false;
			textures[type.c_str()]["debugName"] = textureCI.debugName;
			textures[type.c_str()]["filepath"] = filepaths[0];
		}
	}

	m_CI.filepath = outAssetFile.m_CI.filepath;
}
