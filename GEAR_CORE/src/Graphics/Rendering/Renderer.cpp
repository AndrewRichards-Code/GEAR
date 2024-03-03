#include "gear_core_common.h"

#include "Graphics/Rendering/Renderer.h"
#include "Graphics/Rendering/Passes/TransferPasses.h"
#include "Graphics/Rendering/Passes/MipmapPasses.h"
#include "Graphics/Rendering/Passes/SkyboxPasses.h"
#include "Graphics/Rendering/Passes/ShadowPasses.h"
#include "Graphics/Rendering/Passes/MainRenderPasses.h"
#include "Graphics/Rendering/Passes/PostProcessingPasses.h"
#include "Graphics/Rendering/Passes/OnScreenDisplayPasses.h"
#include "Graphics/Rendering/Passes/SwapchinUIPasses.h"

#include "Graphics/DebugRender.h"
#include "Graphics/Window.h"

#include "UI/UIContext.h"
#include "Objects/Probe.h"
#include "ARC/src/StringConversion.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace objects;

using namespace miru;
using namespace base;

std::map<std::string, Ref<RenderPipeline>> Renderer::s_RenderPipelines;
Renderer::DefaultObjects Renderer::s_DefaultObjects;

Renderer::Renderer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_Context = m_CI.window->GetContext();
	m_Device = m_Context->GetDevice();
	m_SwapchainImageCount = m_CI.window->GetSwapchain()->GetCreateInfo().swapchainCount;
	m_RenderGraph = RenderGraph(m_Context, m_SwapchainImageCount);

	//Present Synchronisation
	Fence::CreateInfo drawFenceCI;
	for (uint32_t i = 0; i < m_SwapchainImageCount; i++)
	{
		drawFenceCI.debugName = "GEAR_CORE_Fence_Renderer_Draw_" + std::to_string(i);
		drawFenceCI.device = m_Device;
		drawFenceCI.signaled = true;
		drawFenceCI.timeout = UINT64_MAX;
		m_DrawFences.emplace_back(Fence::Create(&drawFenceCI));

		Semaphore::CreateInfo semaphoreCI;
		semaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Acquire_" + std::to_string(i);
		semaphoreCI.device = m_Device;
		semaphoreCI.type = Semaphore::Type::BINARY;
		m_AcquireSemaphores.emplace_back(Semaphore::Create(&semaphoreCI));

		semaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Submit_" + std::to_string(i);
		semaphoreCI.device = m_Device;
		semaphoreCI.type = Semaphore::Type::BINARY;
		m_SubmitSemaphores.emplace_back(Semaphore::Create(&semaphoreCI));
	}

	SubmitRenderSurface(m_CI.window->GetRenderSurface());
	InitialiseRenderPipelines(m_RenderSurface);
	DebugRender::Initialise(m_Device);

	//Set Default Objects
	InitialiseDefaultObjects();

	//Set up Post Processing Info
	{
		float zero[sizeof(UniformBufferStructures::BloomInfo)] = { 0 };
		Uniformbuffer<UniformBufferStructures::BloomInfo>::CreateInfo ubCI;
		ubCI.debugName = "GEAR_CORE_Buffer_BloomInfoUB";
		ubCI.device = m_Device;
		ubCI.data = zero;
		m_PostProcessingInfo.bloom.UB = CreateRef<Uniformbuffer<UniformBufferStructures::BloomInfo>>(&ubCI);
		m_PostProcessingInfo.bloom.UB->threshold = 3.0f;
		m_PostProcessingInfo.bloom.UB->upsampleScale = 2.0f;
		m_PostProcessingInfo.bloom.UB->SubmitData();
	}
	{
		float zero[sizeof(UniformBufferStructures::HDRInfo)] = { 0 };
		Uniformbuffer<UniformBufferStructures::HDRInfo>::CreateInfo ubCI;
		ubCI.debugName = "GEAR_CORE_Buffer_HDRSettings";
		ubCI.device = m_Device;
		ubCI.data = zero;
		m_PostProcessingInfo.hdrSettings.UB = CreateRef<Uniformbuffer<UniformBufferStructures::HDRInfo>>(&ubCI);
		m_PostProcessingInfo.hdrSettings.UB->exposure = 1.0f;
		m_PostProcessingInfo.hdrSettings.UB->gammaSpace = static_cast<uint32_t>(ColourSpace::SRGB);
		m_PostProcessingInfo.hdrSettings.UB->SubmitData();
	}
}

Renderer::~Renderer()
{
	m_Context->DeviceWaitIdle();

	UninitialiseDefaultObjects();

	DebugRender::Uninitialise();
	UninitialiseRenderPipelines();
}

void Renderer::InitialiseRenderPipelines(const Ref<RenderSurface>& renderSurface)
{
	if (!s_RenderPipelines.empty())
		return;

	const Image::Format& swapchainFormat = m_CI.window->GetSwapchain()->m_SwapchainImages[0]->GetCreateInfo().format;

	//Filepath, Colour Attachment Formats, Depth Attachment Format
	std::vector<std::tuple<std::string, std::vector<Image::Format>, Image::Format>> filepaths =
	{
		{ "res/pipelines/PBROpaque.grpf",				{ RenderSurface::HDRFormat },	RenderSurface::DepthFormat },
		{ "res/pipelines/Cube.grpf",					{ RenderSurface::HDRFormat },	RenderSurface::DepthFormat },
		{ "res/pipelines/Text.grpf",					{ RenderSurface::SRGBFormat },	Image::Format::UNKNOWN },
		{ "res/pipelines/HDR.grpf",						{ RenderSurface::SRGBFormat },	Image::Format::UNKNOWN },
		{ "res/pipelines/DebugBoxes.grpf",				{ RenderSurface::SRGBFormat },	Image::Format::UNKNOWN },
		{ "res/pipelines/DebugCoordinateAxes.grpf",		{ RenderSurface::SRGBFormat },	Image::Format::UNKNOWN },
		{ "res/pipelines/DebugCopy.grpf",				{ swapchainFormat },			Image::Format::UNKNOWN },
		{ "res/pipelines/DebugShowDepth.grpf",			{ RenderSurface::SRGBFormat },	Image::Format::UNKNOWN },
		{ "res/pipelines/DebugShowDepthCubemap.grpf",	{ RenderSurface::SRGBFormat },	Image::Format::UNKNOWN },
		{ "res/pipelines/Mipmap.grpf",					{},								Image::Format::UNKNOWN },
		{ "res/pipelines/MipmapArray.grpf",				{},								Image::Format::UNKNOWN },
		{ "res/pipelines/EquirectangularToCube.grpf",	{},								Image::Format::UNKNOWN },
		{ "res/pipelines/DiffuseIrradiance.grpf",		{},								Image::Format::UNKNOWN },
		{ "res/pipelines/SpecularIrradiance.grpf",		{},								Image::Format::UNKNOWN },
		{ "res/pipelines/SpecularBRDF_LUT.grpf",		{},								Image::Format::UNKNOWN },
		{ "res/pipelines/BloomPrefilter.grpf",			{},								Image::Format::UNKNOWN },
		{ "res/pipelines/BloomDownsample.grpf",			{},								Image::Format::UNKNOWN },
		{ "res/pipelines/BloomUpsample.grpf",			{},								Image::Format::UNKNOWN },
		{ "res/pipelines/Shadow.grpf",					{},								RenderSurface::DepthFormat },
	};

	RenderPipeline::LoadInfo renderPipelineLI;
	for (auto& filepath : filepaths)
	{
		auto& [filename, colourFormats, depthFormat] = filepath;

		renderPipelineLI.device = m_Device;
		renderPipelineLI.filepath = filename;
		renderPipelineLI.samples = renderSurface->GetCreateInfo().samples;
		renderPipelineLI.colourAttachmentFormats = colourFormats;
		renderPipelineLI.depthAttachmentFormat = depthFormat;
		Ref<RenderPipeline> renderPipeline = CreateRef<RenderPipeline>(&renderPipelineLI);
		s_RenderPipelines[renderPipeline->m_CI.debugName] = renderPipeline;
	}
}

void Renderer::UninitialiseRenderPipelines()
{
	for (auto& pipeline : s_RenderPipelines)
		pipeline.second = nullptr;

	s_RenderPipelines.clear();
}

void Renderer::InitialiseDefaultObjects()
{
	UniformBufferStructures::Lights zero0 = {};
	Uniformbuffer<UniformBufferStructures::Lights>::CreateInfo lightsUBCI;
	lightsUBCI.debugName = "GEAR_CORE_LightUBType_Renderer: Empty";
	lightsUBCI.device = m_Device;
	lightsUBCI.data = &zero0;
	s_DefaultObjects.emptyLightsUB = CreateRef<Uniformbuffer<UniformBufferStructures::Lights>>(&lightsUBCI);

	UniformBufferStructures::ProbeInfo zero1 = {};
	Uniformbuffer<UniformBufferStructures::ProbeInfo>::CreateInfo probeUBCI;
	probeUBCI.debugName = "GEAR_CORE_ProbeUBType_Renderer: Empty";
	probeUBCI.device = m_Device;
	probeUBCI.data = &zero1;
	s_DefaultObjects.emptyProbeUB = CreateRef<Uniformbuffer<UniformBufferStructures::ProbeInfo>>(&probeUBCI);

	uint8_t dataBlack2D[4] = { 0x00, 0x00, 0x00, 0xFF };
	Texture::CreateInfo texCI;
	texCI.debugName = "GEAR_CORE_Texture_Renderer: Black Texture 2D";
	texCI.device = m_Device;
	texCI.dataType = Texture::DataType::DATA;
	texCI.data.data = dataBlack2D;
	texCI.data.size = 4;
	texCI.data.width = 1;
	texCI.data.height = 1;
	texCI.data.depth = 1;
	texCI.mipLevels = 1;
	texCI.arrayLayers = 1;
	texCI.type = miru::base::Image::Type::TYPE_2D;
	texCI.format = miru::base::Image::Format::R8G8B8A8_UNORM;
	texCI.samples = miru::base::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	texCI.usage = miru::base::Image::UsageBit(0);
	texCI.generateMipMaps = false;
	texCI.gammaSpace = GammaSpace::LINEAR;
	s_DefaultObjects.black2DTexture = CreateRef<Texture>(&texCI);

	uint8_t dataBlack2DArray[8] = {
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF
	};
	texCI.debugName = "GEAR_CORE_Texture_Renderer: Black Texture 2D Array";
	texCI.data.data = dataBlack2DArray;
	texCI.data.size = 8;
	texCI.arrayLayers = 2;
	texCI.type = miru::base::Image::Type::TYPE_2D_ARRAY;
	s_DefaultObjects.black2DArrayTexture = CreateRef<Texture>(&texCI);

	uint8_t dataBlackCube[24] = {
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF
	};
	texCI.debugName = "GEAR_CORE_Texture_Renderer: Black Texture Cube";
	texCI.data.data = dataBlackCube;
	texCI.data.size = 24;
	texCI.arrayLayers = 6;
	texCI.type = miru::base::Image::Type::TYPE_CUBE;
	s_DefaultObjects.blackCubeTexture = CreateRef<Texture>(&texCI);

	uint8_t dataBlackCubeArray[48] = {
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF
	};
	texCI.debugName = "GEAR_CORE_Texture_Renderer: Black Texture Cube Array";
	texCI.data.data = dataBlackCubeArray;
	texCI.data.size = 48;
	texCI.arrayLayers = 12;
	texCI.type = miru::base::Image::Type::TYPE_CUBE_ARRAY;
	s_DefaultObjects.blackCubeArrayTexture = CreateRef<Texture>(&texCI);
}

void Renderer::UninitialiseDefaultObjects()
{
	s_DefaultObjects.emptyLightsUB = nullptr;
	s_DefaultObjects.emptyProbeUB = nullptr;
	s_DefaultObjects.black2DTexture = nullptr;
	s_DefaultObjects.black2DArrayTexture = nullptr;
	s_DefaultObjects.blackCubeTexture = nullptr;
	s_DefaultObjects.blackCubeArrayTexture = nullptr;
}

void Renderer::SubmitRenderSurface(const Ref<RenderSurface>& renderSurface)
{
	if (m_RenderSurface != renderSurface)
	{
		m_RenderSurface = renderSurface;
	}
}

void Renderer::SubmitCamera(const Ref<Camera>& camera, uint32_t usage)
{
	m_AllCameras.push_back(camera);

	if (usage == 2)
		m_MainRenderCamera = camera;
	if (usage == 3)
		m_TextCamera = camera;
}

void Renderer::SubmitLight(const Ref<Light>& light)
{
	m_Lights.push_back(light);
}

void Renderer::SubmitSkybox(const Ref<Skybox>& skybox)
{
	if (m_Skybox != skybox)
	{
		m_Skybox = skybox;
	}
}

void Renderer::SubmitModel(const Ref<Model>& obj)
{
	m_ModelQueue.push_back(obj);
}

void Renderer::SubmitTextLine(const Ref<Model>& obj)
{
	m_TextQueue.push_back(obj);
}

void Renderer::SubmitTexturesForUpload(const std::vector<Ref<Texture>>& textures)
{
	m_TextureUploadQueue.insert(m_TextureUploadQueue.end(), textures.begin(), textures.end());
}

void Renderer::SubmitUIContext(ui::UIContext* uiContext)
{
	m_UIContext = uiContext;
}

void Renderer::AcquireNextImage()
{
	m_DrawFences[m_FrameIndex]->Wait();
	m_DrawFences[m_FrameIndex]->Reset();

	if (m_CI.shouldPresent)
	{
		m_CI.window->GetSwapchain()->AcquireNextImage(m_AcquireSemaphores[m_FrameIndex], m_SwapchainImageIndex);
	}
}

void Renderer::Draw()
{
	//Process Models and Textures
	std::vector<Ref<objects::Model>> allQueue;
	allQueue.insert(allQueue.end(), m_ModelQueue.begin(), m_ModelQueue.end());
	allQueue.insert(allQueue.end(), m_TextQueue.begin(), m_TextQueue.end());
	if (m_Skybox)
		allQueue.push_back(m_Skybox->GetModel());
	
	m_AllCameras.push_back(DebugRender::GetCamera());

	std::set<Ref<Texture>> texturesToProcess;
	std::vector<Ref<Texture>> texturesToGenerateMipmaps;

	//Get all unique textures
	for (auto& model : allQueue)
	{
		for (auto& material : model->GetMesh()->GetMaterials())
		{
			for (auto& texture : material->GetTextures())
			{
				if (texture.second)
				{
					texturesToProcess.insert(texture.second);
					if (texture.second->m_GenerateMipMaps && !texture.second->m_GeneratedMipMaps)
					{
						texturesToGenerateMipmaps.push_back(texture.second);
					}
				}
			}
		}
	}
	texturesToProcess.insert(s_DefaultObjects.black2DTexture);
	texturesToProcess.insert(s_DefaultObjects.blackCubeTexture);

	//Reload textures
	if (m_ReloadTextures)
	{
		for (auto& texture : texturesToProcess)
			texture->Reload();
	}
	m_ReloadTextures = false;

	//Build RenderGraph
	m_RenderGraph.Reset(m_FrameIndex);

	//Frame
	{
		GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "GEAR_CORE: Frame " + std::to_string(m_FrameCount));

		//Upload Transfer
		if (!m_AllCameras.empty() || m_Skybox || !m_Lights.empty() || !allQueue.empty())
		{
			passes::TransferPasses::Upload(*this, m_AllCameras, m_Skybox, m_Lights, allQueue);
		}

		//Async Compute Tasks
		{
			GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Async Compute Tasks");
			{
				GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Generate Mipmaps");
				for (const auto& texture : texturesToGenerateMipmaps)
				{
					passes::MipmapPasses::GenerateMipmaps(*this, texture);
				}
			}

			if (m_Skybox && !m_Skybox->m_Generated)
			{
				GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Skybox");
				passes::SkyboxPasses::EquirectangularToCube(*this, m_Skybox);
				passes::SkyboxPasses::GenerateMipmaps(*this, m_Skybox);
				passes::SkyboxPasses::DiffuseIrradiance(*this, m_Skybox);
				passes::SkyboxPasses::SpecularIrradiance(*this, m_Skybox);
				passes::SkyboxPasses::SpecularBRDF_LUT(*this, m_Skybox);

				m_Skybox->m_Generated = true;
			}
		}

		//Shadows
		{
			GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Shadows");
			{
				GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Main");
				for (const auto& light : m_Lights)
				{
					GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, light->GetProbe()->m_CI.debugName);
					passes::ShadowPasses::Main(*this, light);
				}
			}
			{
				GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "DebugShowDepth");
				for (const auto& light : m_Lights)
				{
					GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, light->GetProbe()->m_CI.debugName);
					passes::ShadowPasses::DebugShowDepth(*this, light);
				}
			}
		}

		//Main Render
		{
			bool clear = true;
			
			GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Main Render");
			if (m_MainRenderCamera && m_Skybox)
			{
				GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Skybox");
				passes::MainRenderPasses::Skybox(*this, m_Skybox);
				clear = false;
			}
			if (m_MainRenderCamera && !m_ModelQueue.empty() && (!m_Lights.empty() || m_Skybox))
			{
				GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "PBR Opaque");
				passes::MainRenderPasses::PBROpaque(*this);
				clear = false;
			}

			if (m_MainRenderCamera && clear)
			{
				GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Clear");
				passes::MainRenderPasses::Clear(*this);
			}
		}
		//Post Processing
		{
			GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Post Processing");
			if (m_MainRenderCamera)
			{
				GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "Bloom");
				passes::PostProcessingPasses::Bloom::Prefilter(*this);
				passes::PostProcessingPasses::Bloom::Downsample(*this);
				passes::PostProcessingPasses::Bloom::Upsample(*this);
			}
			if (m_MainRenderCamera)
			{
				passes::PostProcessingPasses::HDRMapping::Main(*this);
			}
		}

		//On Screen Display
		{
			GEAR_RENDER_GRARH_EVENT_SCOPE(m_RenderGraph, "On Screen Display");
			if (m_DebugRendering)
			{
				if (m_TextCamera)
				{
					passes::OnScreenDisplayPasses::Text(*this);
				}
				if (m_MainRenderCamera)
				{
					passes::OnScreenDisplayPasses::CoordinateAxes(*this);
				}
				if (!m_Lights.empty())
				{
					passes::OnScreenDisplayPasses::Boxes(*this);
				}
			}
		}

		//Copy To Swapchain
		if (m_CI.shouldCopyToSwapchian)
		{
			passes::SwapchinUIPasses::CopyToSwapchain(*this);
		}
		//External UI
		if (m_CI.shouldDrawExternalUI)
		{
			passes::SwapchinUIPasses::ExternalUI(*this, m_UIContext);
		}
	}

	//Execute RenderGraph
	m_RenderGraph.Execute();

	//Clean Up
	m_ModelQueue.clear();
	m_TextQueue.clear();
	m_Lights.clear();
	m_AllCameras.clear();
	m_MainRenderCamera = nullptr;
	m_TextCamera = nullptr;
	m_Skybox = nullptr;
}

void Renderer::Present()
{
	const Ref<CommandBuffer>& graphicsCmdBuffer = m_RenderGraph.GetCommandBuffer(CommandPool::QueueType::GRAPHICS);
	const Ref<CommandPool>& graphicsCmdPool = graphicsCmdBuffer->GetCreateInfo().commandPool;
	if (m_CI.shouldPresent)
	{
		CommandBuffer::SubmitInfo submitInfo = { { m_FrameIndex }, { m_AcquireSemaphores[m_FrameIndex] }, {}, { base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT }, { m_SubmitSemaphores[m_FrameIndex] }, {} };
		graphicsCmdBuffer->Submit({ submitInfo }, m_DrawFences[m_FrameIndex]);
		m_CI.window->GetSwapchain()->Present(graphicsCmdPool, m_SubmitSemaphores[m_FrameIndex], m_SwapchainImageIndex);
	}
	else
	{
		CommandBuffer::SubmitInfo submitInfo = { { m_FrameIndex }, {}, {}, {}, {}, {} };
		graphicsCmdBuffer->Submit({ submitInfo }, m_DrawFences[m_FrameIndex]);
	}

	m_FrameIndex = (m_FrameIndex + 1) % m_SwapchainImageCount;
	m_FrameCount++;
}

void Renderer::Execute()
{
	//Acquire Next Image
	{
		AcquireNextImage();
	}
	//Record Present CmdBuffers
	{
		Draw();
	}
	//Present CmdBuffers
	{
		Present();
	}
}

void Renderer::RecompileRenderPipelineShaders()
{
	m_Context->DeviceWaitIdle();
	for (auto& renderPipeline : s_RenderPipelines)
		renderPipeline.second->RecompileShaders();
}

void Renderer::ReloadRenderPipelines()
{
	m_Context->DeviceWaitIdle();
	UninitialiseRenderPipelines();
	InitialiseRenderPipelines(m_RenderSurface);
}

void Renderer::ReloadTextures()
{
	m_Context->DeviceWaitIdle();
	m_ReloadTextures = true;
}
