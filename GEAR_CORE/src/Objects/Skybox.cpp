#include "gear_core_common.h"
#include "Skybox.h"
#include "Graphics/ImageProcessing.h"
#include "stb_image.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

using namespace miru;
using namespace miru::crossplatform;

Skybox::Skybox(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseUBs();
	
	m_Cubemap = m_CI.filepaths.size() == 6;
	m_HDR = stbi_is_hdr(m_CI.filepaths[0].c_str());

	m_TextureCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	m_TextureCI.device = m_CI.device;
	m_TextureCI.dataType = Texture::DataType::FILE;
	m_TextureCI.file.filepaths = m_CI.filepaths.data();
	m_TextureCI.file.count = m_CI.filepaths.size();
	m_TextureCI.mipLevels = 1;
	m_TextureCI.arrayLayers = m_CI.filepaths.size() == 6 ? 6 : 1;
	m_TextureCI.type = m_CI.filepaths.size() == 6 ? Image::Type::TYPE_CUBE : Image::Type::TYPE_2D;
	m_TextureCI.format = m_HDR ? Image::Format::R32G32B32A32_SFLOAT : Image::Format::R8G8B8A8_UNORM;
	m_TextureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_TextureCI.usage = Image::UsageBit(0);
	m_TextureCI.generateMipMaps = false;
	m_Texture = gear::CreateRef<Texture>(&m_TextureCI);

	if (!m_Cubemap)
	{
		m_GeneratedCubemapCI.debugName = "GEAR_CORE_Skybox_GeneratedCubemap: " + m_CI.debugName;
		m_GeneratedCubemapCI.device = m_CI.device;
		m_GeneratedCubemapCI.dataType = Texture::DataType::DATA;
		m_GeneratedCubemapCI.data.data = nullptr;
		m_GeneratedCubemapCI.data.size = 0;
		m_GeneratedCubemapCI.data.width = m_CI.generatedCubemapSize;
		m_GeneratedCubemapCI.data.height = m_CI.generatedCubemapSize;
		m_GeneratedCubemapCI.data.depth = 1;
		m_GeneratedCubemapCI.mipLevels = 1;
		m_GeneratedCubemapCI.arrayLayers = 6;
		m_GeneratedCubemapCI.type = Image::Type::TYPE_CUBE;
		m_GeneratedCubemapCI.format = m_HDR ? Image::Format::R32G32B32A32_SFLOAT : Image::Format::R8G8B8A8_UNORM;
		m_GeneratedCubemapCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		m_GeneratedCubemapCI.usage = Image::UsageBit::STORAGE_BIT;
		m_GeneratedCubemapCI.generateMipMaps = false;
		m_GeneratedCubemap = gear::CreateRef<Texture>(&m_GeneratedCubemapCI);
	}
	else
	{
		m_GeneratedCubemap = nullptr;
	}

	m_MaterialCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	m_MaterialCI.device = m_CI.device;
	m_MaterialCI.pbrTextures = { {Material::TextureType::ALBEDO, m_Texture} };
	m_Material = gear::CreateRef<Material>(&m_MaterialCI);

	m_MeshCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	m_MeshCI.device = m_CI.device;
	m_MeshCI.filepath = "res/obj/cube.fbx";
	m_Mesh = gear::CreateRef<Mesh>(&m_MeshCI);
	m_Mesh->SetOverrideMaterial(0, m_Material);

	m_ModelCI.debugName = "GEAR_CORE_Skybox: " + m_CI.debugName;
	m_ModelCI.device = m_CI.device;
	m_ModelCI.pMesh = m_Mesh;
	m_ModelCI.transform = m_CI.transform;
	m_ModelCI.renderPipelineName = "Cube";
	m_Model = gear::CreateRef<Model>(&m_ModelCI);
	
	Update();
}

Skybox::~Skybox()
{

}

void Skybox::Update()
{
	m_UB->exposure = m_CI.exposure;
	m_UB->gamma = m_CI.gamma;
	m_UB->SubmitData();

	m_Model->GetUB()->modl = TransformToMat4(m_CI.transform);
	m_Model->GetUB()->SubmitData();
}

void Skybox::InitialiseUBs()
{
	float zero[sizeof(SkyboxInfoUB)] = { 0 };
	Uniformbuffer<SkyboxInfoUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Skybox_SkyboxInfo: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_UB = gear::CreateRef<Uniformbuffer<SkyboxInfoUB>>(&ubCI);
}