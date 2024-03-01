#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/SwapchinUIPasses.h"
#include "Graphics/Rendering/Renderer.h"
#include "UI/UIContext.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace passes;

using namespace miru;
using namespace base;

void SwapchinUIPasses::CopyToSwapchain(Renderer& renderer)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	const Ref<Window>& window = renderer.GetWindow();
	uint32_t width = window->GetWidth();
	uint32_t height = window->GetHeight();
	const ImageViewRef& swapchainImageView = window->GetSwapchain()->m_SwapchainImageViews[renderer.GetSwapchainImageIndex()];

	Ref<TaskPassParameters> copyToSwapchainPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["DebugCopy"]);
	copyToSwapchainPassParameters->SetResourceView("sourceImage", ResourceView(renderSurface->GetColourSRGBImageView(), Resource::State::SHADER_READ_ONLY));
	copyToSwapchainPassParameters->AddAttachment(0, ResourceView(swapchainImageView, Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
	copyToSwapchainPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

	renderGraph.AddPass("Copy To Swapchain", copyToSwapchainPassParameters, CommandPool::QueueType::GRAPHICS,
		[](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			cmdBuffer->Draw(frameIndex, 3);
		});
}

void SwapchinUIPasses::ExternalUI(Renderer& renderer, ui::UIContext* uiContext)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<RenderSurface>& renderSurface = renderer.GetRenderSurface();
	const Ref<Window>& window = renderer.GetWindow();
	uint32_t width = window->GetWidth();
	uint32_t height = window->GetHeight();
	const ImageViewRef& swapchainImageView = window->GetSwapchain()->m_SwapchainImageViews[renderer.GetSwapchainImageIndex()];

	Ref<TaskPassParameters> externalUIPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["DebugCopy"]);
	for (const auto& textureID : uiContext->m_TextureIDs)
	{
		externalUIPassParameters->AddResourceViewAtPipelineStage(ResourceView(textureID.first, Resource::State::SHADER_READ_ONLY), PipelineStageBit::FRAGMENT_SHADER_BIT);
	}
	externalUIPassParameters->AddAttachment(0, ResourceView(swapchainImageView, Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.25f, 0.25f, 0.25f, 1.0f });
	externalUIPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

	renderGraph.AddPass("External UI", externalUIPassParameters, CommandPool::QueueType::GRAPHICS,
		[uiContext](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
		{
			if (uiContext)
			{
				uiContext->RenderDrawData(cmdBuffer, frameIndex);
			}
		});
}