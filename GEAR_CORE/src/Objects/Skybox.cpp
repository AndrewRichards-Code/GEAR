#include "gear_core_common.h"
#include "Objects/Skybox.h"

#include "Asset/EditorAssetManager.h"
#include "UI/UIContext.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace project;
using namespace mars;

using namespace miru;
using namespace base;

Skybox::Skybox(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	Texture::CreateInfo textureCI;
	textureCI = {};
	textureCI.debugName = "GEAR_CORE_Skybox_HDR: " + m_CI.debugName;
	textureCI.device = m_CI.device;
	textureCI.imageData = m_CI.textureData->Data;
	textureCI.width = m_CI.textureData->width;
	textureCI.height = m_CI.textureData->height;
	textureCI.depth = m_CI.textureData->depth;
	textureCI.mipLevels = 1;
	textureCI.arrayLayers = 1;
	textureCI.type = Image::Type::TYPE_2D;
	textureCI.format = Image::Format::R32G32B32A32_SFLOAT;
	textureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	textureCI.usage = Image::UsageBit(0);
	textureCI.generateMipMaps = false;
	textureCI.gammaSpace = GammaSpace::LINEAR;
	m_HDRTexture = CreateRef<Texture>(&textureCI);

	textureCI = {};
	textureCI.debugName = "GEAR_CORE_Skybox_GeneratedCubemap: " + m_CI.debugName;
	textureCI.device = m_CI.device;
	textureCI.imageData = {};
	textureCI.width = m_CI.generatedCubemapSize;
	textureCI.height = m_CI.generatedCubemapSize;
	textureCI.depth = 1;
	textureCI.mipLevels = graphics::Texture::MaxMipLevel;
	textureCI.arrayLayers = 6;
	textureCI.type = Image::Type::TYPE_CUBE;
	textureCI.format = Image::Format::R32G32B32A32_SFLOAT;
	textureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	textureCI.usage = Image::UsageBit::STORAGE_BIT;
	textureCI.generateMipMaps = true;
	m_GeneratedCubemap = CreateRef<Texture>(&textureCI);
	
	textureCI = {};
	textureCI.debugName = "GEAR_CORE_Skybox_GeneratedDiffuseCubemap: " + m_CI.debugName;
	textureCI.device = m_CI.device;
	textureCI.imageData = {};
	textureCI.width = m_CI.generatedCubemapSize / 16;
	textureCI.height = m_CI.generatedCubemapSize / 16;
	textureCI.depth = 1;
	textureCI.mipLevels = 1;
	textureCI.arrayLayers = 6;
	textureCI.type = Image::Type::TYPE_CUBE;
	textureCI.format = Image::Format::R32G32B32A32_SFLOAT;
	textureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	textureCI.usage = Image::UsageBit::STORAGE_BIT;
	textureCI.generateMipMaps = false;
	m_GeneratedDiffuseCubemap = CreateRef<Texture>(&textureCI);

	textureCI = {};
	textureCI.debugName = "GEAR_CORE_Skybox_GeneratedSpecularCubemap: " + m_CI.debugName;
	textureCI.device = m_CI.device;
	textureCI.imageData = {};
	textureCI.width = m_CI.generatedCubemapSize;
	textureCI.height = m_CI.generatedCubemapSize;
	textureCI.depth = 1;
	textureCI.mipLevels = graphics::Texture::MaxMipLevel;
	textureCI.arrayLayers = 6;
	textureCI.type = Image::Type::TYPE_CUBE;
	textureCI.format = Image::Format::R32G32B32A32_SFLOAT;
	textureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	textureCI.usage = Image::UsageBit::STORAGE_BIT;
	textureCI.generateMipMaps = true;
	m_GeneratedSpecularCubemap = CreateRef<Texture>(&textureCI);

	textureCI = {};
	textureCI.debugName = "GEAR_CORE_Skybox_GeneratedSpecularBRDF_LUT: " + m_CI.debugName;
	textureCI.device = m_CI.device;
	textureCI.imageData = {};
	textureCI.width = m_CI.generatedCubemapSize;
	textureCI.height = m_CI.generatedCubemapSize;
	textureCI.depth = 1;
	textureCI.mipLevels = 1;
	textureCI.arrayLayers = 1;
	textureCI.type = Image::Type::TYPE_2D;
	textureCI.format = Image::Format::R32G32B32A32_SFLOAT;
	textureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	textureCI.usage = Image::UsageBit::STORAGE_BIT;
	textureCI.generateMipMaps = false;
	m_GeneratedSpecularBRDF_LUT = CreateRef<Texture>(&textureCI);

	Material::CreateInfo materialCI;
	materialCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	materialCI.device = m_CI.device;
	materialCI.pbrTextures = { {Material::TextureType::ALBEDO, m_HDRTexture} };
	m_Material = CreateRef<Material>(&materialCI);

	Mesh::CreateInfo meshCI;
	meshCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	meshCI.device = m_CI.device;
	meshCI.modelData = ui::UIContext::GetUIContext()->GetEditorAssetManager()->Import<utils::ModelLoader::ModelData>(Asset::Type::EXTERNAL_FILE, "res/obj/cube.fbx"); //TODO: Change this!
	m_Mesh = CreateRef<Mesh>(&meshCI);
	m_Mesh->SetOverrideMaterial(0, m_Material);

	Model::CreateInfo modelCI;
	modelCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	modelCI.device = m_CI.device;
	modelCI.mesh = m_Mesh;
	modelCI.renderPipelineName = "Cube";
	m_Model = CreateRef<Model>(&modelCI);
	
	Update(Transform());
}

Skybox::~Skybox()
{

}

void Skybox::Update(const Transform& transform)
{
	if (CreateInfoHasChanged(&m_CI))
	{
		if (m_TextureData!= m_CI.textureData)
		{
			Texture::CreateInfo HDRTextureCI;
			HDRTextureCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
			HDRTextureCI.device = m_CI.device;
			HDRTextureCI.imageData = m_CI.textureData->Data;
			HDRTextureCI.width = m_CI.textureData->width;
			HDRTextureCI.height = m_CI.textureData->height;
			HDRTextureCI.depth = m_CI.textureData->depth;
			HDRTextureCI.mipLevels = 1;
			HDRTextureCI.arrayLayers = 1;
			HDRTextureCI.type = Image::Type::TYPE_2D;
			HDRTextureCI.format = Image::Format::R32G32B32A32_SFLOAT;
			HDRTextureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
			HDRTextureCI.usage = Image::UsageBit(0);
			HDRTextureCI.generateMipMaps = false;
			HDRTextureCI.gammaSpace = GammaSpace::LINEAR;
			m_HDRTexture = CreateRef<Texture>(&HDRTextureCI);

			m_Generated = false;
			m_TextureData= m_CI.textureData;
		}
	}
	if (TransformHasChanged(transform))
	{
		m_Model->GetUB()->modl = TransformToMatrix4(transform);
		m_Model->GetUB()->SubmitData();
	}
}

bool Skybox::CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo)
{
	const CreateInfo& CI = *reinterpret_cast<const CreateInfo*>(pCreateInfo);
	uint64_t newHash = 0;
	newHash ^= core::GetHash(CI.textureData);
	newHash ^= core::GetHash(CI.generatedCubemapSize);
	return CompareCreateInfoHash(newHash);
}