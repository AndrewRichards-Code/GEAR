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

	Ref<TaskPassParameters> skyboxPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["Cube"]);
	skyboxPassParameters->SetResourceView("camera", ResourceView(renderer.GetCamera()->GetCameraUB()));
	skyboxPassParameters->SetResourceView("model", ResourceView(skybox->GetModel()->GetUB()));
	skyboxPassParameters->SetResourceView("skybox", ResourceView(skybox->GetGeneratedCubemap(), Resource::State::SHADER_READ_ONLY));
	skyboxPassParameters->AddAttachmentWithResolve(0, ResourceView(renderSurface->GetMSAAColourImageView(), Resource::State::COLOUR_ATTACHMENT),
		ResourceView(renderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });
	skyboxPassParameters->AddAttachment(0, ResourceView(renderSurface->GetDepthImageView(), Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 1.0f, 0 });
	skyboxPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

	renderGraph.AddPass("Main Render Skybox", skyboxPassParameters, CommandPool::QueueType::GRAPHICS,
		[skybox](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			const Ref<Model>& model = skybox->GetModel();
			cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[0]->GetGPUBufferView() });
			cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[0]->GetGPUBufferView());
			cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[0]->GetCount());
		});
}

void MainRenderPasses::PBROpaque(Renderer& renderer, Ref<objects::Light> light, Ref<objects::Skybox> skybox)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	uint32_t width = renderSurface->GetWidth();
	uint32_t height = renderSurface->GetHeight();

	for (const auto& model : renderer.GetModelQueue())
	{
		for (size_t i = 0; i < model->GetMesh()->GetVertexBuffers().size(); i++)
		{
			Ref<TaskPassParameters> mainRenderPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["PBROpaque"]);
			const Ref<objects::Material>& material = model->GetMesh()->GetMaterial(i);
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
			mainRenderPassParameters->AddAttachmentWithResolve(0, ResourceView(renderSurface->GetMSAAColourImageView(), Resource::State::COLOUR_ATTACHMENT),
				ResourceView(renderSurface->GetColourImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 0.0f });
			mainRenderPassParameters->AddAttachment(0, ResourceView(renderSurface->GetDepthImageView(), Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 1.0f, 0 });
			mainRenderPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

			renderGraph.AddPass("Main Render PBROpaque - " + model->GetDebugName() + ": " + std::to_string(i), mainRenderPassParameters, CommandPool::QueueType::GRAPHICS,
				[model, i](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
				{
					cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[i]->GetGPUBufferView() });
					cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetGPUBufferView());
					cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetCount());
				});
		}
	}
}