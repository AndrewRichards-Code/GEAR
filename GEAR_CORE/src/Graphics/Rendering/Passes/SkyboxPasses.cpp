#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/SkyboxPasses.h"
#include "Graphics/Rendering/Renderer.h"
#include "Objects/Skybox.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace passes;
using namespace objects;

using namespace miru;
using namespace base;

void SkyboxPasses::EquirectangularToCube(Renderer& renderer, Ref<Skybox> skybox)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	Ref<ImageView> generatedCubemap_2DArrayView = renderGraph.CreateImageView(skybox->GetGeneratedCubemap()->GetImage(), Image::Type::TYPE_2D_ARRAY, { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 });
	Ref<TaskPassParameters> equirectangularToCubePassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["EquirectangularToCube"]);
	equirectangularToCubePassParameters->SetResourceView("equirectangularImage", ResourceView(skybox->GetHDRTexture(), DescriptorType::COMBINED_IMAGE_SAMPLER));
	equirectangularToCubePassParameters->SetResourceView("cubeImage", ResourceView(generatedCubemap_2DArrayView, Resource::State::SHADER_READ_WRITE));
	const std::array<uint32_t, 3>& groupCount = equirectangularToCubePassParameters->GetPipeline()->GetCreateInfo().shaders[0]->GetGroupCountXYZ();

	renderGraph.AddPass("Equirectangular To Cube", equirectangularToCubePassParameters, CommandPool::QueueType::COMPUTE,
		[skybox, groupCount](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			uint32_t width = std::max(skybox->GetGeneratedCubemap()->GetWidth() / groupCount[0], uint32_t(1));
			uint32_t height = std::max(skybox->GetGeneratedCubemap()->GetHeight() / groupCount[1], uint32_t(1));
			uint32_t depth = 6 / groupCount[2];
			cmdBuffer->Dispatch(frameIndex, width, height, depth);
		});
}

void SkyboxPasses::GenerateMipmaps(Renderer& renderer, Ref<Skybox> skybox)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<Texture>& texture = skybox->GetGeneratedCubemap();
	const uint32_t& levels = texture->GetCreateInfo().mipLevels;
	const uint32_t& layers = texture->GetCreateInfo().arrayLayers;
	std::vector<Ref<ImageView>> mipImageViews;
	for (uint32_t i = 0; i < levels; i++)
		mipImageViews.push_back(renderGraph.CreateImageView(texture->GetImage(), Image::Type::TYPE_2D_ARRAY, { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers }));
	for (uint32_t i = 0; i < (levels - 1); i++)
	{
		GEAR_RENDER_GRARH_EVENT_SCOPE(renderGraph, "Generate Mipmaps");
		Ref<TaskPassParameters> generateMipMapsPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["MipmapArray"]);
		generateMipMapsPassParameters->SetResourceView("inputImageArray", ResourceView(mipImageViews[std::min(i + 0, levels)], Resource::State::SHADER_READ_ONLY));
		generateMipMapsPassParameters->SetResourceView("outputImageArray", ResourceView(mipImageViews[std::min(i + 1, levels)], Resource::State::SHADER_READ_WRITE));
		const std::array<uint32_t, 3>& groupCount = generateMipMapsPassParameters->GetPipeline()->GetCreateInfo().shaders[0]->GetGroupCountXYZ();

		i++;
		renderGraph.AddPass("Level: " + std::to_string(i), generateMipMapsPassParameters, CommandPool::QueueType::COMPUTE,
			[i, levels, layers, texture, groupCount](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				uint32_t width = std::max((texture->GetWidth() >> i) / groupCount[0], uint32_t(1));
				uint32_t height = std::max((texture->GetHeight() >> i) / groupCount[1], uint32_t(1));
				uint32_t depth = layers / groupCount[2];
				cmdBuffer->Dispatch(frameIndex, width, height, depth);
			});
		i--;
	}
	texture->m_GeneratedMipMaps = true;
}

void SkyboxPasses::DiffuseIrradiance(Renderer& renderer, Ref<Skybox> skybox)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	Ref<ImageView> generatedDiffuseCubemap_2DArrayView = renderGraph.CreateImageView(skybox->GetGeneratedDiffuseCubemap()->GetImage(), Image::Type::TYPE_2D_ARRAY, { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 });
	Ref<TaskPassParameters> diffuseIrradiancePassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["DiffuseIrradiance"]);
	diffuseIrradiancePassParameters->SetResourceView("environment", ResourceView(skybox->GetGeneratedCubemap(), DescriptorType::COMBINED_IMAGE_SAMPLER));
	diffuseIrradiancePassParameters->SetResourceView("diffuseIrradiance", ResourceView(generatedDiffuseCubemap_2DArrayView, Resource::State::SHADER_READ_WRITE));
	const std::array<uint32_t, 3>& groupCount = diffuseIrradiancePassParameters->GetPipeline()->GetCreateInfo().shaders[0]->GetGroupCountXYZ();

	renderGraph.AddPass("Diffuse Irradiance", diffuseIrradiancePassParameters, CommandPool::QueueType::COMPUTE,
		[skybox, groupCount](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			uint32_t width = std::max(skybox->GetGeneratedDiffuseCubemap()->GetWidth() / groupCount[0], uint32_t(1));
			uint32_t height = std::max(skybox->GetGeneratedDiffuseCubemap()->GetHeight() / groupCount[1], uint32_t(1));
			uint32_t depth = 6 / groupCount[2];
			cmdBuffer->Dispatch(frameIndex, width, height, depth);
		});
}

void SkyboxPasses::SpecularIrradiance(Renderer& renderer, Ref<Skybox> skybox)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<Texture>& texture = skybox->GetGeneratedSpecularCubemap();
	const uint32_t& levels = texture->GetCreateInfo().mipLevels;
	std::vector<Ref<ImageView>> generatedSpecularCubemap_2DArrayViews;
	float zero[sizeof(UniformBufferStructures::SpecularIrradianceInfo)] = { 0 };
	std::vector<Ref<Uniformbuffer<UniformBufferStructures::SpecularIrradianceInfo>>> specularIrradianceInfoUBs;
	specularIrradianceInfoUBs.resize(levels);
	for (uint32_t i = 0; i < levels; i++)
	{
		GEAR_RENDER_GRARH_EVENT_SCOPE(renderGraph, "Specular Irradiance");
		generatedSpecularCubemap_2DArrayViews.push_back(renderGraph.CreateImageView(texture->GetImage(), Image::Type::TYPE_2D_ARRAY, { Image::AspectBit::COLOUR_BIT, i, 1, 0, 6 }));
		Uniformbuffer<UniformBufferStructures::SpecularIrradianceInfo>::CreateInfo ubCI;
		ubCI.debugName = "GEAR_CORE_Uniformbuffer_SpecularIrradianceInfoUB";
		ubCI.device = renderer.GetDevice();
		ubCI.data = zero;
		specularIrradianceInfoUBs[i] = CreateRef<Uniformbuffer<UniformBufferStructures::SpecularIrradianceInfo>>(&ubCI);
		specularIrradianceInfoUBs[i]->roughness = float(i) / float(levels);
		specularIrradianceInfoUBs[i]->SubmitData();
		Ref<TaskPassParameters> specularIrradiancePassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["SpecularIrradiance"]);
		specularIrradiancePassParameters->SetResourceView("environment", ResourceView(skybox->GetGeneratedCubemap(), DescriptorType::COMBINED_IMAGE_SAMPLER));
		specularIrradiancePassParameters->SetResourceView("specularIrradiance", ResourceView(generatedSpecularCubemap_2DArrayViews[i], Resource::State::SHADER_READ_WRITE));
		specularIrradiancePassParameters->SetResourceView("specularIrradianceInfo", ResourceView(specularIrradianceInfoUBs[i]));
		const std::array<uint32_t, 3>& groupCount = specularIrradiancePassParameters->GetPipeline()->GetCreateInfo().shaders[0]->GetGroupCountXYZ();

		renderGraph.AddPass("Level: " + std::to_string(i), specularIrradiancePassParameters, CommandPool::QueueType::COMPUTE,
			[texture, i, specularIrradianceInfoUBs, skybox, groupCount](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
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
				uint32_t width = std::max((texture->GetWidth() >> i) / groupCount[0], uint32_t(1));
				uint32_t height = std::max((texture->GetHeight() >> i) / groupCount[1], uint32_t(1));
				uint32_t depth = 6 / groupCount[2];
				cmdBuffer->Dispatch(frameIndex, width, height, depth);
			});
	}
}

void SkyboxPasses::SpecularBRDF_LUT(Renderer& renderer, Ref<Skybox> skybox)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	Ref<TaskPassParameters> specularBRDF_LUT_PassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["SpecularBRDF_LUT"]);
	specularBRDF_LUT_PassParameters->SetResourceView("brdf_lut", ResourceView(skybox->GetGeneratedSpecularBRDF_LUT(), DescriptorType::STORAGE_IMAGE));
	const std::array<uint32_t, 3>& groupCount = specularBRDF_LUT_PassParameters->GetPipeline()->GetCreateInfo().shaders[0]->GetGroupCountXYZ();

	renderGraph.AddPass("Specular BRDF LUT", specularBRDF_LUT_PassParameters, CommandPool::QueueType::COMPUTE,
		[skybox, groupCount](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			uint32_t width = std::max(skybox->GetGeneratedSpecularBRDF_LUT()->GetWidth() / groupCount[0], uint32_t(1));
			uint32_t height = std::max(skybox->GetGeneratedSpecularBRDF_LUT()->GetHeight() / groupCount[1], uint32_t(1));
			uint32_t depth = 1 / groupCount[2];
			cmdBuffer->Dispatch(frameIndex, width, height, depth);
		});
}