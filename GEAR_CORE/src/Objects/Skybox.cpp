#include "gear_core_common.h"
#include "Skybox.h"
#include "Graphics/ImageProcessing.h"
#include "STBI/stb_image.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

using namespace miru;
using namespace miru::crossplatform;

Skybox::Skybox(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if (!(stbi_is_hdr(m_CI.filepath.c_str()) && m_CI.filepath.find(".hdr") != std::string::npos))
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::NOT_SUPPORTED, "%s: is not a supported file format.", m_CI.filepath)
	}

	m_HDRTextureCI.debugName = "GEAR_CORE_Skybox_HDR: " + m_CI.debugName;
	m_HDRTextureCI.device = m_CI.device;
	m_HDRTextureCI.dataType = Texture::DataType::FILE;
	m_HDRTextureCI.file.filepaths = { m_CI.filepath };
	m_HDRTextureCI.mipLevels = 1;
	m_HDRTextureCI.arrayLayers = 1;
	m_HDRTextureCI.type = Image::Type::TYPE_2D;
	m_HDRTextureCI.format = Image::Format::R32G32B32A32_SFLOAT;
	m_HDRTextureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_HDRTextureCI.usage = Image::UsageBit(0);
	m_HDRTextureCI.generateMipMaps = false;
	m_HDRTextureCI.gammaSpace = GammaSpace::LINEAR;
	m_HDRTexture = CreateRef<Texture>(&m_HDRTextureCI);

	m_GeneratedCubemapCI.debugName = "GEAR_CORE_Skybox_GeneratedCubemap: " + m_CI.debugName;
	m_GeneratedCubemapCI.device = m_CI.device;
	m_GeneratedCubemapCI.dataType = Texture::DataType::DATA;
	m_GeneratedCubemapCI.data.data = nullptr;
	m_GeneratedCubemapCI.data.size = 0;
	m_GeneratedCubemapCI.data.width = m_CI.generatedCubemapSize;
	m_GeneratedCubemapCI.data.height = m_CI.generatedCubemapSize;
	m_GeneratedCubemapCI.data.depth = 1;
	m_GeneratedCubemapCI.mipLevels = graphics::Texture::MaxMipLevel;
	m_GeneratedCubemapCI.arrayLayers = 6;
	m_GeneratedCubemapCI.type = Image::Type::TYPE_CUBE;
	m_GeneratedCubemapCI.format = Image::Format::R32G32B32A32_SFLOAT;
	m_GeneratedCubemapCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_GeneratedCubemapCI.usage = Image::UsageBit::STORAGE_BIT;
	m_GeneratedCubemapCI.generateMipMaps = true;
	m_GeneratedCubemap = CreateRef<Texture>(&m_GeneratedCubemapCI);
	

	m_GeneratedDiffuseCubemapCI.debugName = "GEAR_CORE_Skybox_GeneratedDiffuseCubemap: " + m_CI.debugName;
	m_GeneratedDiffuseCubemapCI.device = m_CI.device;
	m_GeneratedDiffuseCubemapCI.dataType = Texture::DataType::DATA;
	m_GeneratedDiffuseCubemapCI.data.data = nullptr;
	m_GeneratedDiffuseCubemapCI.data.size = 0;
	m_GeneratedDiffuseCubemapCI.data.width = m_CI.generatedCubemapSize/16;
	m_GeneratedDiffuseCubemapCI.data.height = m_CI.generatedCubemapSize/16;
	m_GeneratedDiffuseCubemapCI.data.depth = 1;
	m_GeneratedDiffuseCubemapCI.mipLevels = 1;
	m_GeneratedDiffuseCubemapCI.arrayLayers = 6;
	m_GeneratedDiffuseCubemapCI.type = Image::Type::TYPE_CUBE;
	m_GeneratedDiffuseCubemapCI.format = Image::Format::R32G32B32A32_SFLOAT;
	m_GeneratedDiffuseCubemapCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_GeneratedDiffuseCubemapCI.usage = Image::UsageBit::STORAGE_BIT;
	m_GeneratedDiffuseCubemapCI.generateMipMaps = false;
	m_GeneratedDiffuseCubemap = CreateRef<Texture>(&m_GeneratedDiffuseCubemapCI);

	m_GeneratedSpecularCubemapCI.debugName = "GEAR_CORE_Skybox_GeneratedSpecularCubemap: " + m_CI.debugName;
	m_GeneratedSpecularCubemapCI.device = m_CI.device;
	m_GeneratedSpecularCubemapCI.dataType = Texture::DataType::DATA;
	m_GeneratedSpecularCubemapCI.data.data = nullptr;
	m_GeneratedSpecularCubemapCI.data.size = 0;
	m_GeneratedSpecularCubemapCI.data.width = m_CI.generatedCubemapSize;
	m_GeneratedSpecularCubemapCI.data.height = m_CI.generatedCubemapSize;
	m_GeneratedSpecularCubemapCI.data.depth = 1;
	m_GeneratedSpecularCubemapCI.mipLevels = graphics::Texture::MaxMipLevel;
	m_GeneratedSpecularCubemapCI.arrayLayers = 6;
	m_GeneratedSpecularCubemapCI.type = Image::Type::TYPE_CUBE;
	m_GeneratedSpecularCubemapCI.format = Image::Format::R32G32B32A32_SFLOAT;
	m_GeneratedSpecularCubemapCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_GeneratedSpecularCubemapCI.usage = Image::UsageBit::STORAGE_BIT;
	m_GeneratedSpecularCubemapCI.generateMipMaps = true;
	m_GeneratedSpecularCubemap = CreateRef<Texture>(&m_GeneratedSpecularCubemapCI);

	m_GeneratedSpecularBRDF_LUT_CI.debugName = "GEAR_CORE_Skybox_GeneratedSpecularBRDF_LUT: " + m_CI.debugName;
	m_GeneratedSpecularBRDF_LUT_CI.device = m_CI.device;
	m_GeneratedSpecularBRDF_LUT_CI.dataType = Texture::DataType::DATA;
	m_GeneratedSpecularBRDF_LUT_CI.data.data = nullptr;
	m_GeneratedSpecularBRDF_LUT_CI.data.size = 0;
	m_GeneratedSpecularBRDF_LUT_CI.data.width = m_CI.generatedCubemapSize;
	m_GeneratedSpecularBRDF_LUT_CI.data.height = m_CI.generatedCubemapSize;
	m_GeneratedSpecularBRDF_LUT_CI.data.depth = 1;
	m_GeneratedSpecularBRDF_LUT_CI.mipLevels = 1;
	m_GeneratedSpecularBRDF_LUT_CI.arrayLayers = 1;
	m_GeneratedSpecularBRDF_LUT_CI.type = Image::Type::TYPE_2D;
	m_GeneratedSpecularBRDF_LUT_CI.format = Image::Format::R32G32B32A32_SFLOAT;
	m_GeneratedSpecularBRDF_LUT_CI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_GeneratedSpecularBRDF_LUT_CI.usage = Image::UsageBit::STORAGE_BIT;
	m_GeneratedSpecularBRDF_LUT_CI.generateMipMaps = false;
	m_GeneratedSpecularBRDF_LUT = CreateRef<Texture>(&m_GeneratedSpecularBRDF_LUT_CI);

	m_MaterialCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	m_MaterialCI.device = m_CI.device;
	m_MaterialCI.pbrTextures = { {Material::TextureType::ALBEDO, m_HDRTexture} };
	m_Material = CreateRef<Material>(&m_MaterialCI);

	m_MeshCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	m_MeshCI.device = m_CI.device;
	m_MeshCI.filepath = "res/obj/cube.fbx";
	m_Mesh = CreateRef<Mesh>(&m_MeshCI);
	m_Mesh->SetOverrideMaterial(0, m_Material);

	m_ModelCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	m_ModelCI.device = m_CI.device;
	m_ModelCI.pMesh = m_Mesh;
	m_ModelCI.renderPipelineName = "Cube";
	m_Model = CreateRef<Model>(&m_ModelCI);
	
	Update(Transform());
}

Skybox::~Skybox()
{

}

void Skybox::Update(const Transform& transform)
{
	if (CreateInfoHasChanged(&m_CI))
	{
		if (m_HDRTextureCI.file.filepaths[0].compare(m_CI.filepath) != 0) 
		{
			if (!(stbi_is_hdr(m_CI.filepath.c_str()) && m_CI.filepath.find(".hdr") != std::string::npos))
			{
				GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::NOT_SUPPORTED, "%s: is not a supported file format.", m_CI.filepath)
			}

			m_HDRTextureCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
			m_HDRTextureCI.device = m_CI.device;
			m_HDRTextureCI.dataType = Texture::DataType::FILE;
			m_HDRTextureCI.file.filepaths = { m_CI.filepath };
			m_HDRTextureCI.mipLevels = 1;
			m_HDRTextureCI.arrayLayers = 1;
			m_HDRTextureCI.type = Image::Type::TYPE_2D;
			m_HDRTextureCI.format = Image::Format::R32G32B32A32_SFLOAT;
			m_HDRTextureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
			m_HDRTextureCI.usage = Image::UsageBit(0);
			m_HDRTextureCI.generateMipMaps = false;
			m_HDRTextureCI.gammaSpace = GammaSpace::LINEAR;
			m_HDRTexture = CreateRef<Texture>(&m_HDRTextureCI);

			m_Generated = false;
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
	newHash ^= core::GetHash(CI.filepath);
	newHash ^= core::GetHash(CI.generatedCubemapSize);
	return CompareCreateInfoHash(newHash);
}