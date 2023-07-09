#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/OnScreenDisplayPasses.h"
#include "Graphics/Rendering/Renderer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace passes;

using namespace miru;
using namespace base;

void OnScreenDisplayPasses::Text(Renderer& renderer)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();

	for (const auto& model : renderer.GetTextQueue())
	{
		uint32_t width = renderSurface->GetWidth();
		uint32_t height = renderSurface->GetHeight();

		Ref<TaskPassParameters> osdTextPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["Text"]);
		osdTextPassParameters->AddVertexBuffer(ResourceView(model->GetMesh()->GetVertexBuffers()[0]));
		osdTextPassParameters->AddIndexBuffer(ResourceView(model->GetMesh()->GetIndexBuffers()[0]));
		osdTextPassParameters->SetResourceView("textCamera", ResourceView(renderer.GetTextCamera()->GetCameraUB()));
		osdTextPassParameters->SetResourceView("model", ResourceView(model->GetUB()));
		osdTextPassParameters->SetResourceView("fontAtlas", ResourceView(model->GetMesh()->GetMaterial(0)->GetTextures()[objects::Material::TextureType::ALBEDO], DescriptorType::COMBINED_IMAGE_SAMPLER));
		osdTextPassParameters->AddAttachment(0, ResourceView(renderSurface->GetColourSRGBImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::LOAD, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
		osdTextPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

		renderGraph.AddPass("Text" + model->GetDebugName(), osdTextPassParameters, CommandPool::QueueType::GRAPHICS,
			[model](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[0]->GetGPUBufferView() });
				cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[0]->GetGPUBufferView());
				cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[0]->GetCount());
			});
	}
}

void OnScreenDisplayPasses::CoordinateAxes(Renderer& renderer)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	uint32_t width = renderSurface->GetWidth();
	uint32_t height = renderSurface->GetHeight();

	Ref<TaskPassParameters> osdCoordinateAxesPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["DebugCoordinateAxes"]);
	osdCoordinateAxesPassParameters->SetResourceView("camera", ResourceView(renderer.GetCamera()->GetCameraUB()));
	osdCoordinateAxesPassParameters->AddAttachment(0, ResourceView(renderSurface->GetColourSRGBImageView(), Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::LOAD, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
	osdCoordinateAxesPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

	renderGraph.AddPass("Coordinate Axes", osdCoordinateAxesPassParameters, CommandPool::QueueType::GRAPHICS,
		[](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			cmdBuffer->Draw(frameIndex, 6);
		});
}