#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/ShadowPasses.h"
#include "Graphics/Rendering/Renderer.h"
#include "Graphics/DebugRender.h"
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

void ShadowPasses::Main(Renderer& renderer, Ref<Light> light)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<Probe>& probe = light->GetProbe();
	bool omni = probe->m_CI.directionType == Probe::DirectionType::OMNI;
	uint32_t width = probe->m_DepthTexture->GetCreateInfo().data.width;
	uint32_t height = probe->m_DepthTexture->GetCreateInfo().data.height;

	for (const auto& model : renderer.GetModelQueue())
	{
		for (size_t i = 0; i < model->GetMesh()->GetVertexBuffers().size(); i++)
		{
			Ref<TaskPassParameters> shadowPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["Shadow"]);
			shadowPassParameters->SetResourceView("probeInfo", ResourceView(probe->GetUB()));
			shadowPassParameters->SetResourceView("model", ResourceView(model->GetUB()));
			shadowPassParameters->AddAttachment(0, ResourceView(probe->m_DepthTexture, Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 1.0f, 0 });
			shadowPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height), probe->m_DepthTexture->GetCreateInfo().arrayLayers);

			renderGraph.AddPass("Shadow Pass - " + model->GetDebugName() + ": " + std::to_string(i), shadowPassParameters, CommandPool::QueueType::GRAPHICS,
				[model, i, omni](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
				{
					cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[i]->GetGPUBufferView() });
					cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetGPUBufferView());
					cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetCount(), omni ? 6 : 1);
				});
		}
	}
}

void ShadowPasses::DebugShowDepth(Renderer& renderer, Ref<Light> light) 
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const Ref<Probe>& probe = light->GetProbe();
	if (probe->m_RenderDebugView)
	{
		Ref<Texture>& debugShowDepthTexture = probe->m_DebugTexture;
		if (!debugShowDepthTexture)
		{
			Texture::CreateInfo tCI;
			tCI.debugName = "GEAR_CORE_Texture_Probe_DebugTexture: " + probe->m_CI.debugName;
			tCI.device = renderer.GetDevice();
			tCI.dataType = Texture::DataType::DATA;
			tCI.data = { nullptr, 0, 256, 256, 1 };
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

		Ref<TaskPassParameters> debugShowDepthParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()[renderPipelineName]);
		debugShowDepthParameters->SetResourceView("debugCamera", ResourceView(DebugRender::GetCamera()->GetCameraUB()));
		debugShowDepthParameters->SetResourceView(omni ? "cubemap" : "image2D", ResourceView(probe->m_DepthTexture, DescriptorType::COMBINED_IMAGE_SAMPLER));
		debugShowDepthParameters->AddAttachment(0, ResourceView(debugShowDepthTexture, Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 1.0f });
		debugShowDepthParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height));

		renderGraph.AddPass("Shadow - " + renderPipelineName, debugShowDepthParameters, CommandPool::QueueType::GRAPHICS,
			[](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				cmdBuffer->Draw(frameIndex, 6);
			});
	}
}