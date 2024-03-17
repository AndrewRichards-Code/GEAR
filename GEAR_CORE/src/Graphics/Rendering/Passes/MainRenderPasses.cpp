#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/MainRenderPasses.h"
#include "Graphics/Rendering/Renderer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"

#include "Objects/Light.h"
#include "Objects/Probe.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace passes;
using namespace objects;

using namespace miru;
using namespace base;

void MainRenderPasses::Clear(Renderer& renderer)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	uint32_t width = renderSurface->GetWidth();
	uint32_t height = renderSurface->GetHeight();
	bool msaa = renderSurface->GetAntiAliasing() > Image::SampleCountBit::SAMPLE_COUNT_1_BIT;

	Ref<TaskPassParameters> skyboxPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["Cube"]);
	if (msaa)
		skyboxPassParameters->AddAttachmentWithResolve(0, ResourceView(renderSurface->GetMSAAColourImageView(), Resource::State::COLOUR_ATTACHMENT),
			ResourceView(renderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });
	else
		skyboxPassParameters->AddAttachment(0, ResourceView(renderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });
	skyboxPassParameters->AddAttachment(0, ResourceView(renderSurface->GetDepthImageView(), Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f,  0 });
	skyboxPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

	renderGraph.AddPass("Clear Colour and Depth", skyboxPassParameters, CommandPool::QueueType::GRAPHICS,
		[](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
		});
}

void MainRenderPasses::Skybox(Renderer& renderer, Ref<objects::Skybox> skybox)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	uint32_t width = renderSurface->GetWidth();
	uint32_t height = renderSurface->GetHeight();
	const Ref<Mesh>& mesh = skybox->GetModel()->GetMesh();
	bool msaa = renderSurface->GetAntiAliasing() > Image::SampleCountBit::SAMPLE_COUNT_1_BIT;

	Ref<TaskPassParameters> skyboxPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["Cube"]);
	skyboxPassParameters->AddVertexBuffer(ResourceView(mesh->GetVertexBuffers()[0]));
	skyboxPassParameters->AddIndexBuffer(ResourceView(mesh->GetIndexBuffers()[0]));
	skyboxPassParameters->SetResourceView("camera", ResourceView(renderer.GetCamera()->GetCameraUB()));
	skyboxPassParameters->SetResourceView("model", ResourceView(skybox->GetModel()->GetUB()));
	skyboxPassParameters->SetResourceView("skybox", ResourceView(skybox->GetGeneratedCubemap(), Resource::State::SHADER_READ_ONLY));
	if (msaa)
		skyboxPassParameters->AddAttachmentWithResolve(0, ResourceView(renderSurface->GetMSAAColourImageView(), Resource::State::COLOUR_ATTACHMENT),
			ResourceView(renderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });
	else
		skyboxPassParameters->AddAttachment(0, ResourceView(renderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });
	skyboxPassParameters->AddAttachment(0, ResourceView(renderSurface->GetDepthImageView(), Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f,  0 });
	skyboxPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

	renderGraph.AddPass("Skybox: " + skybox->m_CI.debugName, skyboxPassParameters, CommandPool::QueueType::GRAPHICS,
		[mesh](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			cmdBuffer->BindVertexBuffers(frameIndex, { mesh->GetVertexBuffers()[0]->GetGPUBufferView() });
			cmdBuffer->BindIndexBuffer(frameIndex, mesh->GetIndexBuffers()[0]->GetGPUBufferView());
			cmdBuffer->DrawIndexed(frameIndex, mesh->GetIndexBuffers()[0]->GetCount());
		});
}

void MainRenderPasses::PBROpaque(Renderer& renderer)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	uint32_t width = renderSurface->GetWidth();
	uint32_t height = renderSurface->GetHeight();
	bool msaa = renderSurface->GetAntiAliasing() > Image::SampleCountBit::SAMPLE_COUNT_1_BIT;

	const std::vector<Ref<objects::Light>>& lights = renderer.GetLights();
	const Ref<objects::Skybox>& skybox = renderer.GetSkybox();

	bool hasLights = !lights.empty();
	bool omni = false;
	Ref<objects::Probe> probe = nullptr;
	if (hasLights)
	{
		probe = lights[0]->GetProbe();
		omni =  probe->m_CI.directionType == Probe::DirectionType::OMNI;
	}

	const Renderer::DefaultObjects& defaultObject = renderer.GetDefaultObjects();

	const Ref<Uniformbuffer<UniformBufferStructures::Lights>>& lightsUB = hasLights ? lights[0]->GetUB() : defaultObject.emptyLightsUB;

	const Ref<Texture>& diffuseIrradiance = skybox ? skybox->GetGeneratedDiffuseCubemap() : defaultObject.blackCubeTexture;
	const Ref<Texture>& specularIrradiance = skybox ? skybox->GetGeneratedSpecularCubemap() : defaultObject.blackCubeTexture;
	const Ref<Texture>& specularBRDF_LUT = skybox ? skybox->GetGeneratedSpecularBRDF_LUT() : defaultObject.black2DTexture;

	const Ref<Texture>& shadowMap2DArray = probe && !omni ? probe->m_DepthTexture : defaultObject.black2DArrayTexture;
	const Ref<Texture>& shadowMapCube = probe && omni ? probe->m_DepthTexture : defaultObject.blackCubeTexture;
	const Ref<Uniformbuffer<UniformBufferStructures::ProbeInfo>>& probeInfoUB = probe ? probe->GetUB() : defaultObject.emptyProbeUB;

	for (const auto& model : renderer.GetModelQueue())
	{
		GEAR_RENDER_GRARH_EVENT_SCOPE(renderGraph, model->GetDebugName());
		const Ref<Mesh>& mesh = model->GetMesh();
		for (size_t i = 0; i < mesh->GetVertexBuffers().size(); i++)
		{
			Ref<TaskPassParameters> mainRenderPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["PBROpaque"]);
			const Ref<objects::Material>& material = mesh->GetMaterial(i);
			auto& materialTextures = material->GetTextures();
			mainRenderPassParameters->AddVertexBuffer(ResourceView(mesh->GetVertexBuffers()[i]));
			mainRenderPassParameters->AddIndexBuffer(ResourceView(mesh->GetIndexBuffers()[i]));
			mainRenderPassParameters->SetResourceView("camera", ResourceView(renderer.GetCamera()->GetCameraUB()));
			mainRenderPassParameters->SetResourceView("lights", ResourceView(lightsUB));
			mainRenderPassParameters->SetResourceView("diffuseIrradiance", ResourceView(diffuseIrradiance, DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("specularIrradiance", ResourceView(specularIrradiance, DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("specularBRDF_LUT", ResourceView(specularBRDF_LUT, DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("shadowMap2DArray", ResourceView(shadowMap2DArray, DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("shadowMapCube", ResourceView(shadowMapCube, DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("probeInfo", ResourceView(probeInfoUB));
			mainRenderPassParameters->SetResourceView("model", ResourceView(model->GetUB()));
			mainRenderPassParameters->SetResourceView("pbrConstants", ResourceView(material->GetUB()));
			mainRenderPassParameters->SetResourceView("normal", ResourceView(materialTextures[Material::TextureType::NORMAL], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("albedo", ResourceView(materialTextures[Material::TextureType::ALBEDO], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("metallic", ResourceView(materialTextures[Material::TextureType::METALLIC], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("roughness", ResourceView(materialTextures[Material::TextureType::ROUGHNESS], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("ambientOcclusion", ResourceView(materialTextures[Material::TextureType::AMBIENT_OCCLUSION], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("emissive", ResourceView(materialTextures[Material::TextureType::EMISSIVE], DescriptorType::COMBINED_IMAGE_SAMPLER));
			if (msaa)
				mainRenderPassParameters->AddAttachmentWithResolve(0, ResourceView(renderSurface->GetMSAAColourImageView(), Resource::State::COLOUR_ATTACHMENT),
					ResourceView(renderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });
			else
				mainRenderPassParameters->AddAttachment(0, ResourceView(renderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });

			mainRenderPassParameters->AddAttachment(0, ResourceView(renderSurface->GetDepthImageView(), Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0 });
			mainRenderPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

			renderGraph.AddPass("Sub Mesh: " + std::to_string(i), mainRenderPassParameters, CommandPool::QueueType::GRAPHICS,
				[mesh, i](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
				{
					cmdBuffer->BindVertexBuffers(frameIndex, { mesh->GetVertexBuffers()[i]->GetGPUBufferView() });
					cmdBuffer->BindIndexBuffer(frameIndex, mesh->GetIndexBuffers()[i]->GetGPUBufferView());
					cmdBuffer->DrawIndexed(frameIndex, mesh->GetIndexBuffers()[i]->GetCount());
				});
		}
	}
}