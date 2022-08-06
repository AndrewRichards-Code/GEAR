#include "gear_core_common.h"

#include "Graphics/Rendering/Renderer.h"
#include "Graphics/DebugRender.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/Window.h"

#include "Objects/Probe.h"
#include "UI/UIContext.h"
#include "ARC/src/StringConversion.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace objects;

using namespace miru;
using namespace base;

std::map<std::string, Ref<RenderPipeline>> Renderer::s_RenderPipelines;

Renderer::Renderer(CreateInfo* pCreateInfo)
	:renderGraph(pCreateInfo->window->GetContext(), pCreateInfo->window->GetSwapchain()->GetCreateInfo().swapchainCount)
{
	m_CI = *pCreateInfo;

	m_Context = m_CI.window->GetContext();
	m_Device = m_Context->GetDevice();
	m_SwapchainImageCount = m_CI.window->GetSwapchain()->GetCreateInfo().swapchainCount;

	//Present Synchronisation
	for (uint32_t i = 0; i < m_SwapchainImageCount; i++)
	{
		m_DrawFenceCI.debugName = "GEAR_CORE_Fence_Renderer_Draw_" + std::to_string(i);
		m_DrawFenceCI.device = m_Device;
		m_DrawFenceCI.signaled = true;
		m_DrawFenceCI.timeout = UINT64_MAX;
		m_DrawFences.emplace_back(Fence::Create(&m_DrawFenceCI));
	}

	m_AcquireSemaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Acquire";
	m_AcquireSemaphoreCI.device = m_Device;
	m_AcquireSemaphoreCI.type = Semaphore::Type::BINARY;
	m_AcquireSemaphore = Semaphore::Create(&m_AcquireSemaphoreCI);

	m_SubmitSemaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Submit";
	m_SubmitSemaphoreCI.device = m_Device;
	m_SubmitSemaphoreCI.type = Semaphore::Type::BINARY;
	m_SubmitSemaphore = Semaphore::Create(&m_SubmitSemaphoreCI);

	SubmitRenderSurface(m_CI.window->GetRenderSurface());
	InitialiseRenderPipelines(m_RenderSurface);

	//Set Default Objects
	float zero0[sizeof(UniformBufferStructures::Lights)] = { 0 };
	Uniformbuffer<UniformBufferStructures::Lights>::CreateInfo lightsUBCI;
	lightsUBCI.debugName = "GEAR_CORE_LightUBType_Renderer: Empty";
	lightsUBCI.device = m_Device;
	lightsUBCI.data = zero0;
	m_EmptyLightsUB = CreateRef<Uniformbuffer<UniformBufferStructures::Lights>>(&lightsUBCI);

	uint8_t dataBlack[4] = { 0x00, 0x00, 0x00, 0xFF };
	Texture::CreateInfo texCI;
	texCI.debugName = "GEAR_CORE_Texture_Renderer: Black Texture 2D";
	texCI.device = m_Device;
	texCI.dataType = Texture::DataType::DATA;
	texCI.data.data = dataBlack;
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
	m_Black2DTexture = CreateRef<Texture>(&texCI);

	uint8_t dataBlackCube[24] = { 
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0xFF
	};
	texCI.debugName = "GEAR_CORE_Texture_Renderer: Black Texture cube";
	texCI.data.data = dataBlackCube;
	texCI.data.size = 24;
	texCI.arrayLayers = 6;
	texCI.type = miru::base::Image::Type::TYPE_CUBE;
	m_BlackCubeTexture = CreateRef<Texture>(&texCI);
}

Renderer::~Renderer()
{
	m_Context->DeviceWaitIdle();

	UninitialiseRenderPipelines();
}

void Renderer::InitialiseRenderPipelines(const Ref<RenderSurface>& renderSurface)
{
	if (!s_RenderPipelines.empty())
		return;

	DebugRender::Initialise(m_Device);

	const Image::Format& swapchainFormat = m_CI.window->GetSwapchain()->m_SwapchainImages[0]->GetCreateInfo().format;
	
	//Filepath, RenderPass Index, Subpass Index
	std::vector<std::tuple<std::string, std::vector<Image::Format>, Image::Format>> filepaths =
	{
		{ "res/pipelines/PBROpaque.grpf",				{ RenderSurface::HDRFormat },	RenderSurface::DepthFormat },
		{ "res/pipelines/Cube.grpf",					{ RenderSurface::HDRFormat },	RenderSurface::DepthFormat },
		{ "res/pipelines/Text.grpf",					{ RenderSurface::SRGBFormat },	Image::Format::UNKNOWN },
		{ "res/pipelines/HDR.grpf",						{ RenderSurface::SRGBFormat },	Image::Format::UNKNOWN },
		{ "res/pipelines/DebugCoordinateAxes.grpf",		{ RenderSurface::SRGBFormat },	Image::Format::UNKNOWN },
		{ "res/pipelines/DebugCopy.grpf",				{ swapchainFormat },			Image::Format::UNKNOWN},
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
	DebugRender::Uninitialise();

	for (auto& pipeline : s_RenderPipelines)
		pipeline.second = nullptr;

	s_RenderPipelines.clear();
}

void Renderer::SubmitRenderSurface(const Ref<RenderSurface>& renderSurface)
{ 
	if (m_RenderSurface != renderSurface)
	{
		m_RenderSurface = renderSurface;
	}
}

void Renderer::SubmitCamera(const Ref<objects::Camera>& camera, uint32_t usage)
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

void Renderer::SubmitUIContext(ui::UIContext* uiContext)
{
	m_UIContext = uiContext;
}

void Renderer::AcquireNextImage()
{
	if (m_CI.shouldPresent)
	{
		m_CI.window->GetSwapchain()->AcquireNextImage(m_AcquireSemaphore, m_FrameIndex);
	}

	m_DrawFences[m_FrameIndex]->Wait();
	m_DrawFences[m_FrameIndex]->Reset();
}

void Renderer::Draw()
{
	renderGraph.Reset();

	std::vector<Ref<objects::Model>> allQueue;
	allQueue.insert(allQueue.end(), m_ModelQueue.begin(), m_ModelQueue.end());
	allQueue.insert(allQueue.end(), m_TextQueue.begin(), m_TextQueue.end());
	if (m_Skybox)
		allQueue.push_back(m_Skybox->GetModel());

	std::set<Ref<Texture>> texturesToProcess;
	std::vector<Ref<Texture>> texturesToGenerateMipmaps;

	//Get all unique textures
	for (auto& model : allQueue)
	{
		for (auto& material : model->GetMesh()->GetMaterials())
		{
			for (auto& texture : material->GetTextures())
			{
				texturesToProcess.insert(texture.second);
				if (texture.second->m_GenerateMipMaps && !texture.second->m_GeneratedMipMaps)
				{
					texturesToGenerateMipmaps.push_back(texture.second);
				}
			}
		}
	}
	texturesToProcess.insert(m_Black2DTexture);
	texturesToProcess.insert(m_BlackCubeTexture);

	//Reload textures
	if (m_ReloadTextures)
	{
		for (auto& texture : texturesToProcess)
			texture->Reload();
	}
	m_ReloadTextures = false;

	//Upload Transfer
	if (!m_AllCameras.empty() || m_Skybox || !m_Lights.empty() || !allQueue.empty())
	{
		Ref<TransferPassParameters> uploadPassParameters = CreateRef<TransferPassParameters>();
		for (const auto& camera : m_AllCameras)
		{
			if (camera && camera->GetUpdateGPUFlag())
			{
				uploadPassParameters->AddResource(camera->GetCameraUB());
				uploadPassParameters->AddResource(camera->GetHDRInfoUB());
				camera->ResetUpdateGPUFlag();
			}
		}
		if (m_Skybox && m_Skybox->GetUpdateGPUFlag())
		{
			uploadPassParameters->AddResource(m_Skybox->GetHDRTexture());
			m_Skybox->ResetUpdateGPUFlag();
		}
		for (const auto& light : m_Lights)
		{
			if (light && light->GetUpdateGPUFlag())
			{
				uploadPassParameters->AddResource(light->GetUB());
				light->ResetUpdateGPUFlag();
			}
		}
		for (const auto& model : allQueue)
		{
			if (model && model->GetUpdateGPUFlag())
			{
				for (const auto& vb : model->GetMesh()->GetVertexBuffers())
					uploadPassParameters->AddResource(vb);
				for (const auto& ib : model->GetMesh()->GetIndexBuffers())
					uploadPassParameters->AddResource(ib);

				uploadPassParameters->AddResource(model->GetUB());
				model->ResetUpdateGPUFlag();

				for (const auto& material : model->GetMesh()->GetMaterials())
				{
					uploadPassParameters->AddResource(material->GetUB());
					for (const auto& texture : material->GetTextures())
						uploadPassParameters->AddResource(texture.second);

					material->ResetUpdateGPUFlag();
				}
			}
		}
		renderGraph.AddPass("Upload Transfer", uploadPassParameters, CommandPool::QueueType::TRANSFER, nullptr);
	}

	//Async Compute Task
	{
		//Generate Mip Maps
		for (const auto& texture : texturesToGenerateMipmaps)
		{
			const uint32_t& levels = texture->GetCreateInfo().mipLevels;
			const uint32_t& layers = texture->GetCreateInfo().arrayLayers;
			const std::string& name = texture->GetCreateInfo().debugName;
			std::vector<Ref<ImageView>> mipImageViews;

			for (uint32_t i = 0; i < levels; i++)
				mipImageViews.push_back(renderGraph.CreateImageView(texture->GetImage(), layers > 1 ? Image::Type::TYPE_2D_ARRAY : Image::Type::TYPE_2D, {Image::AspectBit::COLOUR_BIT, i, 1, 0, layers}));
		
			Ref<TaskPassParameters> generateMipMapsPassParameters;
			for (uint32_t i = 0; i < (levels - 1); i++)
			{
				if (layers > 1)
				{
					generateMipMapsPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["MipmapArray"]);
					generateMipMapsPassParameters->SetResource("inputImageArray", Resource(mipImageViews[std::min(i + 0, levels)], Resource::State::SHADER_READ_ONLY));
					generateMipMapsPassParameters->SetResource("outputImageArray", Resource(mipImageViews[std::min(i + 1, levels)], Resource::State::SHADER_READ_WRITE));
				}
				else
				{
					generateMipMapsPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["Mipmap"]);
					generateMipMapsPassParameters->SetResource("inputImage", Resource(mipImageViews[std::min(i + 0, levels)], Resource::State::SHADER_READ_ONLY));
					generateMipMapsPassParameters->SetResource("outputImage", Resource(mipImageViews[std::min(i + 1, levels)], Resource::State::SHADER_READ_WRITE));
				}
			
				i++;
				renderGraph.AddPass("Generate Mip Maps  - " + name + " - Level: " + std::to_string(i), generateMipMapsPassParameters, CommandPool::QueueType::COMPUTE,
					[generateMipMapsPassParameters, i, layers, texture](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
					{
							uint32_t width = std::max((texture->GetWidth() >> i) / 8, uint32_t(1));
							uint32_t height = std::max((texture->GetHeight() >> i) / 8, uint32_t(1));
							uint32_t depth = layers;
							cmdBuffer->Dispatch(frameIndex, width, height, depth);
					});
				i--;
			}
			texture->m_GeneratedMipMaps = true;
		}

		if (m_Skybox && !m_Skybox->m_Generated)
		{
			//Skybox: Equirectangular To Cube
			{
				Ref<ImageView> generatedCubemap_2DArrayView = renderGraph.CreateImageView(m_Skybox->GetGeneratedCubemap()->GetImage(), Image::Type::TYPE_2D_ARRAY, { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 });
				Ref<TaskPassParameters> equirectangularToCubePassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["EquirectangularToCube"]);
				equirectangularToCubePassParameters->SetResource("equirectangularImage", Resource(m_Skybox->GetHDRTexture(), DescriptorType::COMBINED_IMAGE_SAMPLER));
				equirectangularToCubePassParameters->SetResource("cubeImage", Resource(generatedCubemap_2DArrayView, Resource::State::SHADER_READ_WRITE));

				renderGraph.AddPass("Skybox: Equirectangular To Cube", equirectangularToCubePassParameters, CommandPool::QueueType::COMPUTE,
					[equirectangularToCubePassParameters, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
					{
						uint32_t width = std::max(m_Skybox->GetGeneratedCubemap()->GetWidth() / 32, uint32_t(1));
						uint32_t height = std::max(m_Skybox->GetGeneratedCubemap()->GetHeight() / 32, uint32_t(1));
						uint32_t depth = 6;
						cmdBuffer->Dispatch(frameIndex, width, height, depth);
					});
			}
#if 0
			//Skybox: Generate Mip Maps
			{ 
				const Ref<Texture>& texture = m_Skybox->GetGeneratedCubemap();
				const uint32_t& levels = texture->GetCreateInfo().mipLevels;
				const uint32_t& layers = texture->GetCreateInfo().arrayLayers;

				std::vector<Ref<ImageView>> mipImageViews;
				for (uint32_t i = 0; i < levels; i++)
					mipImageViews.push_back(renderGraph.CreateImageView(texture->GetImage(), Image::Type::TYPE_2D_ARRAY, { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers }));
				
				for (uint32_t i = 0; i < (levels - 1); i++)
				{
					Ref<TaskPassParameters> generateMipMapsPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["MipmapArray"]);
					generateMipMapsPassParameters->SetResource("inputImageArray", Resource(mipImageViews[std::min(i + 0, levels)], Resource::State::SHADER_READ_ONLY));
					generateMipMapsPassParameters->SetResource("outputImageArray", Resource(mipImageViews[std::min(i + 1, levels)], Resource::State::SHADER_READ_WRITE));
					
					i++;
					renderGraph.AddPass("Skybox: Generate Mip Maps - Level: " + std::to_string(i), generateMipMapsPassParameters, CommandPool::QueueType::COMPUTE,
						[generateMipMapsPassParameters, i, levels, layers, texture](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
						{
							uint32_t width = std::max((texture->GetWidth() >> i) / 8, uint32_t(1));
							uint32_t height = std::max((texture->GetHeight() >> i) / 8, uint32_t(1));
							uint32_t depth = layers;
							cmdBuffer->Dispatch(frameIndex, width, height, depth);

							texture->m_GeneratedMipMaps = true;
						});
					i--;
				}
			}
			
			//Skybox: Diffuse Irradiance
			{
				Ref<ImageView> generatedDiffuseCubemap_2DArrayView = renderGraph.CreateImageView(m_Skybox->GetGeneratedDiffuseCubemap()->GetImage(), Image::Type::TYPE_2D_ARRAY, { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 });
				Ref<TaskPassParameters> diffuseIrradiancePassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["DiffuseIrradiance"]);
				diffuseIrradiancePassParameters->SetResource("environment", Resource(m_Skybox->GetGeneratedCubemap(), DescriptorType::COMBINED_IMAGE_SAMPLER));
				diffuseIrradiancePassParameters->SetResource("diffuseIrradiance", Resource(generatedDiffuseCubemap_2DArrayView, Resource::State::SHADER_READ_WRITE));

				renderGraph.AddPass("Skybox: Diffuse Irradiance", diffuseIrradiancePassParameters, CommandPool::QueueType::COMPUTE,
					[diffuseIrradiancePassParameters, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
					{
						uint32_t width = std::max(m_Skybox->GetGeneratedDiffuseCubemap()->GetWidth() / 32, uint32_t(1));
						uint32_t height = std::max(m_Skybox->GetGeneratedDiffuseCubemap()->GetHeight() / 32, uint32_t(1));
						uint32_t depth = 6;
						cmdBuffer->Dispatch(frameIndex, width, height, depth);
					});
			}

			//Skybox: Specular Irradiance
			{
				const Ref<Texture>& texture = m_Skybox->GetGeneratedSpecularCubemap();
				const uint32_t& levels = texture->GetCreateInfo().mipLevels;

				std::vector<Ref<ImageView>> generatedSpecularCubemap_2DArrayViews;

				float zero[sizeof(UniformBufferStructures::SpecularIrradianceInfo)] = { 0 };
				std::vector<Ref<Uniformbuffer<UniformBufferStructures::SpecularIrradianceInfo>>> specularIrradianceInfoUBs;
				specularIrradianceInfoUBs.resize(levels);

				for (uint32_t i = 0; i < levels; i++)
				{
					generatedSpecularCubemap_2DArrayViews.push_back(renderGraph.CreateImageView(texture->GetImage(), Image::Type::TYPE_2D_ARRAY, { Image::AspectBit::COLOUR_BIT, i, 1, 0, 6 }));

					Uniformbuffer<UniformBufferStructures::SpecularIrradianceInfo>::CreateInfo ubCI;
					ubCI.debugName = "GEAR_CORE_Uniformbuffer_SpecularIrradianceInfoUB";
					ubCI.device = m_Device;
					ubCI.data = zero;
					specularIrradianceInfoUBs[i] = CreateRef<Uniformbuffer<UniformBufferStructures::SpecularIrradianceInfo>>(&ubCI);
					specularIrradianceInfoUBs[i]->roughness = float(i) / float(levels);
					specularIrradianceInfoUBs[i]->SubmitData();

					Ref<TaskPassParameters> specularIrradiancePassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["SpecularIrradiance"]);
					specularIrradiancePassParameters->SetResource("environment", Resource(m_Skybox->GetGeneratedCubemap(), DescriptorType::COMBINED_IMAGE_SAMPLER));
					specularIrradiancePassParameters->SetResource("specularIrradiance", Resource(generatedSpecularCubemap_2DArrayViews[i], Resource::State::SHADER_READ_WRITE));
					specularIrradiancePassParameters->SetResource("specularIrradianceInfo", Resource(specularIrradianceInfoUBs[i]));

					renderGraph.AddPass("Skybox: Specular Irradiance - Level: " + std::to_string(i), specularIrradiancePassParameters, CommandPool::QueueType::COMPUTE,
						[specularIrradiancePassParameters, texture, i, specularIrradianceInfoUBs, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
						{
							cmdBuffer->CopyBuffer(frameIndex, specularIrradianceInfoUBs[i]->GetCPUBuffer(), specularIrradianceInfoUBs[i]->GetGPUBuffer(), { {0, 0, specularIrradianceInfoUBs[i]->GetSize()} });
							if (GraphicsAPI::IsD3D12())
							{
								Barrier::CreateInfo bCI;
								bCI.type = Barrier::Type::BUFFER;
								bCI.srcAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
								bCI.dstAccess = Barrier::AccessBit::UNIFORM_READ_BIT;
								bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
								bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
								bCI.buffer = specularIrradianceInfoUBs[i]->GetGPUBuffer();
								bCI.offset = 0;
								bCI.size = specularIrradianceInfoUBs[i]->GetSize();
								Ref<Barrier> b = Barrier::Create(&bCI);
								cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, { b });
							};

							uint32_t width = std::max((texture->GetWidth() >> i) / 32, uint32_t(1));
							uint32_t height = std::max((texture->GetHeight() >> i) / 32, uint32_t(1));
							uint32_t depth = 6;
							cmdBuffer->Dispatch(frameIndex, width, height, depth);
						});
				}
			}

			//Skybox: Specular BRDF LUT
			{
				Ref<TaskPassParameters> specularBRDF_LUT_PassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["SpecularBRDF_LUT"]);
				specularBRDF_LUT_PassParameters->SetResource("brdf_lut", Resource(m_Skybox->GetGeneratedSpecularBRDF_LUT(), DescriptorType::STORAGE_IMAGE));

				renderGraph.AddPass("Skybox: Specular BRDF LUT", specularBRDF_LUT_PassParameters, CommandPool::QueueType::COMPUTE,
					[specularBRDF_LUT_PassParameters, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
					{
						uint32_t width = std::max(m_Skybox->GetGeneratedSpecularBRDF_LUT()->GetWidth() / 32, uint32_t(1));
						uint32_t height = std::max(m_Skybox->GetGeneratedSpecularBRDF_LUT()->GetHeight() / 32, uint32_t(1));
						uint32_t depth = 1;
						cmdBuffer->Dispatch(frameIndex, width, height, depth);
					});
			}
#endif
			m_Skybox->m_Generated = true;
		}
	}

	//Shadow Pass
	if (!m_Lights.empty())
	{
		//Shadow Pass
		{
			const Ref<Probe>& probe = m_Lights[0]->GetProbe();
			bool omni = probe->m_CI.directionType == Probe::DirectionType::OMNI;
			uint32_t width = probe->m_DepthTexture->GetCreateInfo().data.width;
			uint32_t height = probe->m_DepthTexture->GetCreateInfo().data.height;

			for (const auto& model : m_ModelQueue)
			{
				for (size_t i = 0; i < model->GetMesh()->GetVertexBuffers().size(); i++)
				{
					Ref<TaskPassParameters> shadowPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["Shadow"]);
					shadowPassParameters->SetResource("probeInfo", Resource(probe->GetUB()));
					shadowPassParameters->SetResource("model", Resource(model->GetUB()));
					shadowPassParameters->AddAttachment(0, Resource(probe->m_DepthTexture, Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 1.0f, 0 });
					shadowPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height), probe->m_DepthTexture->GetCreateInfo().arrayLayers);

					renderGraph.AddPass("Shadow Pass - " + model->GetDebugName() + ": " + std::to_string(i), shadowPassParameters, CommandPool::QueueType::GRAPHICS,
						[shadowPassParameters, model, i, omni, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
						{
							cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[i]->GetGPUBufferView() });
							cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetGPUBufferView());
							cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetCount(), omni ? 6 : 1);
						});
				}
			}
		}

		//Shadow - DebugShowDepth
		{
			const Ref<Probe>& probe = m_Lights[0]->GetProbe();
			if (probe->m_RenderDebugView)
			{
				Ref<Texture>& debugShowDepthTexture = probe->m_DebugTexture;
				if (!debugShowDepthTexture)
				{
					Texture::CreateInfo tCI;
					tCI.debugName = "GEAR_CORE_Texture_Probe_DebugTexture: " + probe->m_CI.debugName;
					tCI.device = m_Device;
					tCI.dataType = Texture::DataType::DATA;
					tCI.data = { nullptr, 0, 0, 0, 0 };
					tCI.mipLevels = 1;
					tCI.arrayLayers = 1;
					tCI.type = Image::Type::TYPE_2D;
					tCI.format = RenderSurface::SRGBFormat;
					tCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
					tCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::SAMPLED_BIT;
					tCI.generateMipMaps = false;
					tCI.gammaSpace = GammaSpace::SRGB;
					debugShowDepthTexture = CreateRef<Texture>(&tCI);
				}
				bool omni = probe->m_CI.directionType == Probe::DirectionType::OMNI;
				uint32_t width = debugShowDepthTexture->GetCreateInfo().data.width;
				uint32_t height = debugShowDepthTexture->GetCreateInfo().data.height;

				std::string renderPipelineName = omni ? "DebugShowDepthCubemap" : "DebugShowDepth";

				Ref<TaskPassParameters> debugShowDepthParameters = CreateRef<TaskPassParameters>(s_RenderPipelines[renderPipelineName]);
				debugShowDepthParameters->SetResource("debugCamera", Resource(DebugRender::GetCamera()->GetCameraUB()));
				debugShowDepthParameters->SetResource(omni ? "cubemap" : "image2D", Resource(probe->m_DepthTexture, DescriptorType::COMBINED_IMAGE_SAMPLER));
				debugShowDepthParameters->AddAttachment(0, Resource(debugShowDepthTexture, Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 1.0f });
				debugShowDepthParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

				renderGraph.AddPass("Shadow - " + renderPipelineName, debugShowDepthParameters, CommandPool::QueueType::GRAPHICS,
					[debugShowDepthParameters](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
					{
						cmdBuffer->Draw(frameIndex, 6);
					});
			}
		}
	}

	//Main Render PBROpaque
	if (m_MainRenderCamera && !m_Lights.empty() && m_Skybox && !m_ModelQueue.empty())
	{
		uint32_t width = m_RenderSurface->GetWidth();
		uint32_t height = m_RenderSurface->GetHeight();

		for (const auto& model : m_ModelQueue)
		{
			for (size_t i = 0; i < model->GetMesh()->GetVertexBuffers().size(); i++)
			{
				Ref<TaskPassParameters> mainRenderPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["PBROpaque"]);
				const Ref<objects::Material>& material = model->GetMesh()->GetMaterial(i);
				mainRenderPassParameters->SetResource("camera", Resource(m_MainRenderCamera->GetCameraUB()));
				mainRenderPassParameters->SetResource("lights", Resource(m_Lights[0]->GetUB()));
				mainRenderPassParameters->SetResource("diffuseIrradiance", Resource(m_Skybox->GetGeneratedDiffuseCubemap(), DescriptorType::COMBINED_IMAGE_SAMPLER));
				mainRenderPassParameters->SetResource("specularIrradiance", Resource(m_Skybox->GetGeneratedSpecularCubemap(), DescriptorType::COMBINED_IMAGE_SAMPLER));
				mainRenderPassParameters->SetResource("specularBRDF_LUT", Resource(m_Skybox->GetGeneratedSpecularBRDF_LUT(), DescriptorType::COMBINED_IMAGE_SAMPLER));
				mainRenderPassParameters->SetResource("model", Resource(model->GetUB()));
				mainRenderPassParameters->SetResource("pbrConstants", Resource(material->GetUB()));
				mainRenderPassParameters->SetResource("normal", Resource(material->GetTextures()[Material::TextureType::NORMAL], DescriptorType::COMBINED_IMAGE_SAMPLER));
				mainRenderPassParameters->SetResource("albedo", Resource(material->GetTextures()[Material::TextureType::ALBEDO], DescriptorType::COMBINED_IMAGE_SAMPLER));
				mainRenderPassParameters->SetResource("metallic", Resource(material->GetTextures()[Material::TextureType::METALLIC], DescriptorType::COMBINED_IMAGE_SAMPLER));
				mainRenderPassParameters->SetResource("roughness", Resource(material->GetTextures()[Material::TextureType::ROUGHNESS], DescriptorType::COMBINED_IMAGE_SAMPLER));
				mainRenderPassParameters->SetResource("ambientOcclusion", Resource(material->GetTextures()[Material::TextureType::AMBIENT_OCCLUSION], DescriptorType::COMBINED_IMAGE_SAMPLER));
				mainRenderPassParameters->SetResource("emissive", Resource(material->GetTextures()[Material::TextureType::EMISSIVE], DescriptorType::COMBINED_IMAGE_SAMPLER));
				mainRenderPassParameters->AddAttachmentWithResolve(0, Resource(m_RenderSurface->GetMSAAColourImageView(), Resource::State::COLOUR_ATTACHMENT),
					Resource(m_RenderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });
				mainRenderPassParameters->AddAttachment(0, Resource(m_RenderSurface->GetDepthImageView(), Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 1.0f, 0 });
				mainRenderPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

				renderGraph.AddPass("Main Render PBROpaque - " + model->GetDebugName() + ": " + std::to_string(i), mainRenderPassParameters, CommandPool::QueueType::GRAPHICS,
					[mainRenderPassParameters, model, i, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
					{
						cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[i]->GetGPUBufferView() });
						cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetGPUBufferView());
						cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetCount());
					});
			}
		}
	}

	//Main Render Skybox
	if (m_MainRenderCamera && m_Skybox)
	{
		uint32_t width = m_RenderSurface->GetWidth();
		uint32_t height = m_RenderSurface->GetHeight();

		Ref<TaskPassParameters> skyboxPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["Cube"]);
		skyboxPassParameters->SetResource("camera", Resource(m_MainRenderCamera->GetCameraUB()));
		skyboxPassParameters->SetResource("model", Resource(m_Skybox->GetModel()->GetUB()));
		skyboxPassParameters->SetResource("skybox", Resource(m_Skybox->GetGeneratedCubemap(), Resource::State::SHADER_READ_ONLY));
		skyboxPassParameters->AddAttachmentWithResolve(0, Resource(m_RenderSurface->GetMSAAColourImageView(), Resource::State::COLOUR_ATTACHMENT),
			Resource(m_RenderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });
		skyboxPassParameters->AddAttachment(0, Resource(m_RenderSurface->GetDepthImageView(), Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 1.0f, 0 });
		skyboxPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

		renderGraph.AddPass("Main Render Skybox", skyboxPassParameters, CommandPool::QueueType::GRAPHICS,
			[skyboxPassParameters, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				const Ref<Model>& model = m_Skybox->GetModel();
				cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[0]->GetGPUBufferView() });
				cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[0]->GetGPUBufferView());
				cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[0]->GetCount());
			});
	}

#if 0
	//Post Processing Bloom Prefilter - Downsample - Upsample
	if (m_MainRenderCamera)
	{
		//Prefilter
		const ImageViewRef& colourImageView = m_RenderSurface->GetColourImageView();
		const ImageRef& colourImage = colourImageView->GetCreateInfo().image;

		uint32_t width = colourImage->GetCreateInfo().width;
		uint32_t height = colourImage->GetCreateInfo().height;
		uint32_t minSize = std::min(colourImage->GetCreateInfo().width, colourImage->GetCreateInfo().height);
		uint32_t levels = std::max(static_cast<uint32_t>(log2(static_cast<double>(minSize / 8))), uint32_t(1));

		ImageRef prefilterOutputImage = renderGraph.CreateImage({ Image::Type::TYPE_2D, Image::Format::R16G16B16A16_SFLOAT, width, height, 1, levels, 1, Image::SampleCountBit::SAMPLE_COUNT_1_BIT, RenderGraph::ImageDesc::UsageBit::SHADER_READ_WRITE }, "Bloom_Prefilter_Output");
		ImageViewRef prefilterOutputImageView = renderGraph.CreateImageView(prefilterOutputImage, Image::Type::TYPE_2D, { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 });

		float zero[sizeof(UniformBufferStructures::BloomInfo)] = { 0 };
		Uniformbuffer<UniformBufferStructures::BloomInfo>::CreateInfo ubCI;
		ubCI.debugName = "GEAR_CORE_Buffer_BloomInfoUB";
		ubCI.device = m_Device;
		ubCI.data = zero;
		Ref<Uniformbuffer<UniformBufferStructures::BloomInfo>> bloomInfoUB = CreateRef<Uniformbuffer<UniformBufferStructures::BloomInfo>>(&ubCI);
		bloomInfoUB->threshold = 3.0f;
		bloomInfoUB->upsampleScale = 2.0f;
		bloomInfoUB->SubmitData();

		Ref<TaskPassParameters> bloomPreFilterPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["BloomPreFilter"]);
		bloomPreFilterPassParameters->SetResource("inputImage", Resource(colourImageView, Resource::State::SHADER_READ_WRITE));
		bloomPreFilterPassParameters->SetResource("outputImage", Resource(prefilterOutputImageView, Resource::State::SHADER_READ_WRITE));
		bloomPreFilterPassParameters->SetResource("bloomInfo", Resource(bloomInfoUB));

		renderGraph.AddPass("Post Processing Bloom Prefilter", bloomPreFilterPassParameters, CommandPool::QueueType::COMPUTE,
			[bloomPreFilterPassParameters, bloomInfoUB, width, height](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				cmdBuffer->CopyBuffer(frameIndex, bloomInfoUB->GetCPUBuffer(), bloomInfoUB->GetGPUBuffer(), { { 0, 0, bloomInfoUB->GetSize() } });
				uint32_t _width = std::max(width / 8, uint32_t(1));
				uint32_t _height = std::max(height / 8, uint32_t(1));
				uint32_t _depth = 1;
				_width += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
				_height += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
				cmdBuffer->Dispatch(frameIndex, _width, _height, _depth);
			});

		//Downsample
		Sampler::CreateInfo samplerCI;
		samplerCI.debugName = "GEAR_CORE_Sampler_Bloom";
		samplerCI.device = m_Device;
		samplerCI.magFilter = Sampler::Filter::LINEAR;
		samplerCI.minFilter = Sampler::Filter::LINEAR;
		samplerCI.mipmapMode = Sampler::MipmapMode::NEAREST;
		samplerCI.addressModeU = Sampler::AddressMode::CLAMP_TO_EDGE;
		samplerCI.addressModeV = Sampler::AddressMode::CLAMP_TO_EDGE;
		samplerCI.addressModeW = Sampler::AddressMode::CLAMP_TO_EDGE;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.anisotropyEnable = false;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.compareEnable = false;
		samplerCI.compareOp = CompareOp::NEVER;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 0.0f;
		samplerCI.borderColour = Sampler::BorderColour::FLOAT_TRANSPARENT_BLACK;
		samplerCI.unnormalisedCoordinates = false;
		SamplerRef sampler = Sampler::Create(&samplerCI);
		std::vector<ImageViewRef> bloomImageViews;
		
		for (uint32_t i = 0; i < levels; i++)
		{
			bloomImageViews.push_back(renderGraph.CreateImageView(prefilterOutputImage, Image::Type::TYPE_2D, { Image::AspectBit::COLOUR_BIT, i, 1, 0, 1 }));

			Ref<TaskPassParameters> bloomDownsamplePassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["BloomDownsample"]);
			bloomDownsamplePassParameters->SetResource("inputImageRO", Resource(bloomImageViews[std::min(i + 0, levels)], sampler));
			bloomDownsamplePassParameters->SetResource("outputImage", Resource(bloomImageViews[std::min(i + 1, levels)], Resource::State::SHADER_READ_WRITE));

			renderGraph.AddPass("Post Processing Bloom Downsample: " + std::to_string(i), bloomDownsamplePassParameters, CommandPool::QueueType::COMPUTE,
				[bloomDownsamplePassParameters, i, prefilterOutputImage, width, height](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
				{
					Image::Layout imageLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;

					uint32_t _width = std::max((width >> i) / 8, uint32_t(1));
					uint32_t _height = std::max((height >> i) / 8, uint32_t(1));
					uint32_t _depth = 1;
					_width += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
					_height += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
					cmdBuffer->Dispatch(frameIndex, _width, _height, _depth);
				});
		}

		//Upsample
		bloomImageViews[0] = colourImageView;
		for (uint32_t i = levels - 1; i >= 1; i--)
		{
			Ref<TaskPassParameters> bloomUpsamplePassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["BloomUpsample"]);
			bloomUpsamplePassParameters->SetResource("inputImageRO", Resource(bloomImageViews[i + 0], sampler));
			bloomUpsamplePassParameters->SetResource("outputImage", Resource(bloomImageViews[i - 1], Resource::State::SHADER_READ_WRITE));
			bloomUpsamplePassParameters->SetResource("bloomInfo", Resource(bloomInfoUB));

			renderGraph.AddPass("Post Processing Bloom Upsample: " + std::to_string(i), bloomUpsamplePassParameters, CommandPool::QueueType::COMPUTE,
				[bloomUpsamplePassParameters, i, bloomImageViews, width, height](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
				{
					uint32_t _width = std::max((width >> (i - 1)) / 8, uint32_t(1));
					uint32_t _height = std::max((height >> (i - 1)) / 8, uint32_t(1));
					uint32_t _depth = 1;
					_width += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
					_height += 1;		//Add 1 for 'overscanning' the image thus dealing with odd number dispatch groups.
					cmdBuffer->Dispatch(frameIndex, _width, _height, _depth);
				});
		}
	}
#endif

	//HDR Mapping
	if (m_MainRenderCamera)
	{
		uint32_t width = m_RenderSurface->GetWidth();
		uint32_t height = m_RenderSurface->GetHeight();

		Ref<TaskPassParameters> hdrMappingPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["HDR"]);
		hdrMappingPassParameters->SetResource("hdrInfo", Resource(m_MainRenderCamera->GetHDRInfoUB()));
		hdrMappingPassParameters->SetResource("hdrInput", Resource(m_RenderSurface->GetColourImageView(), Resource::State::SHADER_READ_ONLY));
		hdrMappingPassParameters->AddAttachment(0, Resource(m_RenderSurface->GetColourSRGBImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
		hdrMappingPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

		renderGraph.AddPass("HDR Mapping", hdrMappingPassParameters, CommandPool::QueueType::GRAPHICS,
			[hdrMappingPassParameters, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				cmdBuffer->Draw(frameIndex, 3);
			});
	}

	//OSD Text
	if (m_TextCamera)
	{
		for (const auto& model : m_TextQueue)
		{
			uint32_t width = m_RenderSurface->GetWidth();
			uint32_t height = m_RenderSurface->GetHeight();

			Ref<TaskPassParameters> osdTextPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["Text"]);
			osdTextPassParameters->SetResource("model", Resource(model->GetUB()));
			osdTextPassParameters->SetResource("fontAtlas", Resource(model->GetMesh()->GetMaterial(0)->GetTextures()[Material::TextureType::ALBEDO], DescriptorType::COMBINED_IMAGE_SAMPLER));
			osdTextPassParameters->AddAttachment(0, Resource(m_RenderSurface->GetColourSRGBImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::LOAD, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
			osdTextPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

			renderGraph.AddPass("OSD Text" + model->GetDebugName(), osdTextPassParameters, CommandPool::QueueType::GRAPHICS,
				[osdTextPassParameters, model, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
				{
					cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[0]->GetGPUBufferView() });
					cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[0]->GetGPUBufferView());
					cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[0]->GetCount());
				});
		}
	}
	
	//OSD Coordinate Axes
	if (m_MainRenderCamera)
	{
		uint32_t width = m_RenderSurface->GetWidth();
		uint32_t height = m_RenderSurface->GetHeight();

		Ref<TaskPassParameters> osdCoordinateAxesPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["DebugCoordinateAxes"]);
		osdCoordinateAxesPassParameters->SetResource("camera", Resource(m_MainRenderCamera->GetCameraUB()));
		osdCoordinateAxesPassParameters->AddAttachment(0, Resource(m_RenderSurface->GetColourSRGBImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::LOAD, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
		osdCoordinateAxesPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

		renderGraph.AddPass("OSD Coordinate Axes", osdCoordinateAxesPassParameters, CommandPool::QueueType::GRAPHICS,
			[osdCoordinateAxesPassParameters, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				cmdBuffer->Draw(frameIndex, 6);
			});
	}

	//Copy To Swapchain
	if (m_CI.shouldCopyToSwapchian)
	{
		uint32_t width = m_CI.window->GetWidth();
		uint32_t height = m_CI.window->GetHeight();
		const ImageViewRef& swapchainImageView = m_CI.window->GetSwapchain()->m_SwapchainImageViews[m_FrameIndex];

		Ref<TaskPassParameters> copyToSwapchainPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["DebugCopy"]);
		copyToSwapchainPassParameters->SetResource("sourceImage", Resource(m_RenderSurface->GetColourSRGBImageView(), Resource::State::SHADER_READ_ONLY));
		copyToSwapchainPassParameters->AddAttachment(0, Resource(swapchainImageView, Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
		copyToSwapchainPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

		renderGraph.AddPass("Copy To Swapchain", copyToSwapchainPassParameters, CommandPool::QueueType::GRAPHICS,
			[copyToSwapchainPassParameters, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				cmdBuffer->Draw(frameIndex, 3);
			});
	}

	//External UI
	if (m_CI.shouldDrawExternalUI)
	{
		uint32_t width = m_CI.window->GetWidth();
		uint32_t height = m_CI.window->GetHeight();
		const ImageViewRef& swapchainImageView = m_CI.window->GetSwapchain()->m_SwapchainImageViews[m_FrameIndex];

		Ref<TaskPassParameters> externalUIPassParameters = CreateRef<TaskPassParameters>(s_RenderPipelines["DebugCopy"]);
		externalUIPassParameters->SetResource("sourceImage", Resource(m_RenderSurface->GetColourSRGBImageView(), Resource::State::SHADER_READ_ONLY));
		externalUIPassParameters->AddAttachment(0, Resource(swapchainImageView, Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
		externalUIPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

		renderGraph.AddPass("External UI", externalUIPassParameters, CommandPool::QueueType::GRAPHICS,
			[externalUIPassParameters, this](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				if (m_UIContext)
				{
					m_UIContext->RenderDrawData(cmdBuffer, frameIndex);
				}
			});
	}

	renderGraph.Execute(m_FrameIndex);

	m_ModelQueue.clear();
	m_TextQueue.clear();
	m_Lights.clear();
	m_AllCameras.clear();
}

void Renderer::Present()
{
	const Ref<CommandBuffer>& graphicsCmdBuffer = renderGraph.GetCommandBuffer(CommandPool::QueueType::GRAPHICS);
	const Ref<CommandPool>& graphicsCmdPool = graphicsCmdBuffer->GetCreateInfo().commandPool;
	if (m_CI.shouldPresent)
	{
		CommandBuffer::SubmitInfo submitInfo = { { m_FrameIndex }, { m_AcquireSemaphore }, {}, { base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT }, { m_SubmitSemaphore }, {} };
		graphicsCmdBuffer->Submit({ submitInfo }, m_DrawFences[m_FrameIndex]);
		m_CI.window->GetSwapchain()->Present(graphicsCmdPool, m_SubmitSemaphore, m_FrameIndex);
	}
	else
	{
		CommandBuffer::SubmitInfo submitInfo = { { m_FrameIndex }, {}, {}, {}, {}, {} };
		graphicsCmdBuffer->Submit({ submitInfo }, m_DrawFences[m_FrameIndex]);
		
		//Increment m_FrameIndex as AcquireNextImage() will not update it.
		m_FrameIndex = (m_FrameIndex + 1) % m_SwapchainImageCount;
	}

	m_FrameCount++;
}

void Renderer::Execute()
{
	/*Ref<debug::RenderDoc> rd = ref_cast<debug::RenderDoc>(GraphicsAPI::GetGraphicsDebugger());
	bool frameStart = false;
	if (m_Skybox && !m_Skybox->m_Generated)
	{
		rd->m_RenderDocApi->StartFrameCapture(0, 0);
		frameStart = true;
	}*/

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

	/*if (frameStart)
	{
		rd->m_RenderDocApi->EndFrameCapture(0, 0);
	}*/
}

void Renderer::RecompileRenderPipelineShaders()
{
	m_Context->DeviceWaitIdle();
	for (auto& renderPipeline : s_RenderPipelines)
		renderPipeline.second->RecompileShaders();
}

void Renderer::ReloadTextures()
{
	m_Context->DeviceWaitIdle();
	m_ReloadTextures = true;
}
