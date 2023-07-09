#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/MainRenderPasses.h"
#include "Graphics/Rendering/Renderer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"

#include "Objects/Light.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace passes;
using namespace objects;

using namespace miru;
using namespace base;

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

	renderGraph.AddPass("Skybox", skyboxPassParameters, CommandPool::QueueType::GRAPHICS,
		[mesh](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			cmdBuffer->BindVertexBuffers(frameIndex, { mesh->GetVertexBuffers()[0]->GetGPUBufferView() });
			cmdBuffer->BindIndexBuffer(frameIndex, mesh->GetIndexBuffers()[0]->GetGPUBufferView());
			cmdBuffer->DrawIndexed(frameIndex, mesh->GetIndexBuffers()[0]->GetCount());
		});
}

void MainRenderPasses::PBROpaque(Renderer& renderer, Ref<objects::Light> light, Ref<objects::Skybox> skybox)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	uint32_t width = renderSurface->GetWidth();
	uint32_t height = renderSurface->GetHeight();
	bool msaa = renderSurface->GetAntiAliasing() > Image::SampleCountBit::SAMPLE_COUNT_1_BIT;

	for (const auto& model : renderer.GetModelQueue())
	{
		GEAR_RENDER_GRARH_EVENT_SCOPE(renderGraph, model->GetDebugName());
		const Ref<Mesh>& mesh = model->GetMesh();
		for (size_t i = 0; i < mesh->GetVertexBuffers().size(); i++)
		{
			Ref<TaskPassParameters> mainRenderPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["PBROpaque"]);
			const Ref<objects::Material>& material = mesh->GetMaterial(i);
			mainRenderPassParameters->AddVertexBuffer(ResourceView(mesh->GetVertexBuffers()[i]));
			mainRenderPassParameters->AddIndexBuffer(ResourceView(mesh->GetIndexBuffers()[i]));
			mainRenderPassParameters->SetResourceView("camera", ResourceView(renderer.GetCamera()->GetCameraUB()));
			mainRenderPassParameters->SetResourceView("lights", ResourceView(light->GetUB()));
			mainRenderPassParameters->SetResourceView("diffuseIrradiance", ResourceView(skybox->GetGeneratedDiffuseCubemap(), DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("specularIrradiance", ResourceView(skybox->GetGeneratedSpecularCubemap(), DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("specularBRDF_LUT", ResourceView(skybox->GetGeneratedSpecularBRDF_LUT(), DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("model", ResourceView(model->GetUB()));
			mainRenderPassParameters->SetResourceView("pbrConstants", ResourceView(material->GetUB()));
			mainRenderPassParameters->SetResourceView("normal", ResourceView(material->GetTextures()[Material::TextureType::NORMAL], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("albedo", ResourceView(material->GetTextures()[Material::TextureType::ALBEDO], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("metallic", ResourceView(material->GetTextures()[Material::TextureType::METALLIC], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("roughness", ResourceView(material->GetTextures()[Material::TextureType::ROUGHNESS], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("ambientOcclusion", ResourceView(material->GetTextures()[Material::TextureType::AMBIENT_OCCLUSION], DescriptorType::COMBINED_IMAGE_SAMPLER));
			mainRenderPassParameters->SetResourceView("emissive", ResourceView(material->GetTextures()[Material::TextureType::EMISSIVE], DescriptorType::COMBINED_IMAGE_SAMPLER));
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