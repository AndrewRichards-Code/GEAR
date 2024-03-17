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
	bool shadowCascades = probe->m_CI.shadowCascades > 1;
	const uint32_t& width = probe->m_DepthTexture->GetCreateInfo().data.width;
	const uint32_t& height = probe->m_DepthTexture->GetCreateInfo().data.height;
	const uint32_t& arrayLayers = probe->m_DepthTexture->GetCreateInfo().arrayLayers;

	for (const auto& model : renderer.GetModelQueue())
	{
		GEAR_RENDER_GRARH_EVENT_SCOPE(renderGraph, model->GetDebugName());
		const Ref<Mesh>& mesh = model->GetMesh();
		for (size_t i = 0; i < mesh->GetVertexBuffers().size(); i++)
		{
			Ref<TaskPassParameters> shadowPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()[shadowCascades ? "ShadowCascades" : "ShadowSingle"]);
			shadowPassParameters->AddVertexBuffer(ResourceView(mesh->GetVertexBuffers()[i]));
			shadowPassParameters->AddIndexBuffer(ResourceView(mesh->GetIndexBuffers()[i]));
			shadowPassParameters->SetResourceView("probeInfo", ResourceView(probe->GetUB()));
			shadowPassParameters->SetResourceView("model", ResourceView(model->GetUB()));
			shadowPassParameters->AddAttachment(0, ResourceView(probe->m_DepthTexture, Resource::State::DEPTH_STENCIL_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0 });
			shadowPassParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height), arrayLayers, (shadowCascades ? 0b1111 : 0));

			renderGraph.AddPass("Sub Mesh: " + std::to_string(i), shadowPassParameters, CommandPool::QueueType::GRAPHICS,
				[mesh, i, arrayLayers](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
				{
					cmdBuffer->BindVertexBuffers(frameIndex, { mesh->GetVertexBuffers()[i]->GetGPUBufferView() });
					cmdBuffer->BindIndexBuffer(frameIndex, mesh->GetIndexBuffers()[i]->GetGPUBufferView());
					cmdBuffer->DrawIndexed(frameIndex, mesh->GetIndexBuffers()[i]->GetCount(), arrayLayers);
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
			tCI.arrayLayers = Probe::MaxShadowCascades;
			tCI.type = Image::Type::TYPE_2D_ARRAY;
			tCI.format = RenderSurface::SRGBFormat;
			tCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
			tCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::SAMPLED_BIT;
			tCI.generateMipMaps = false;
			tCI.gammaSpace = GammaSpace::SRGB;
			debugShowDepthTexture = CreateRef<Texture>(&tCI);
		}
		bool omni = probe->m_CI.directionType == Probe::DirectionType::OMNI;
		const uint32_t& width = debugShowDepthTexture->GetCreateInfo().data.width;
		const uint32_t& height = debugShowDepthTexture->GetCreateInfo().data.height;

		std::string renderPipelineName = omni ? "DebugShowDepthCubemap" : "DebugShowDepth";

		Ref<TaskPassParameters> debugShowDepthParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()[renderPipelineName]);
		if (omni)
			debugShowDepthParameters->SetResourceView("debugCamera", ResourceView(DebugRender::GetCamera()->GetCameraUB()));
		debugShowDepthParameters->SetResourceView("debugProbeInfo", ResourceView(DebugRender::GetDebugProbeInfo()));
		debugShowDepthParameters->SetResourceView(omni ? "cubemap" : "image2DArray", ResourceView(probe->m_DepthTexture, DescriptorType::COMBINED_IMAGE_SAMPLER));
		debugShowDepthParameters->AddAttachment(0, ResourceView(debugShowDepthTexture, Resource::State::COLOUR_ATTACHMENT), RenderPass::AttachmentLoadOp::CLEAR, RenderPass::AttachmentStoreOp::STORE, { 0.0f, 0.0f, 0.0f, 1.0f });
		debugShowDepthParameters->SetRenderArea(TaskPassParameters::CreateScissor(width, height), 1, 0b1111);

		renderGraph.AddPass(renderPipelineName, debugShowDepthParameters, CommandPool::QueueType::GRAPHICS,
			[](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
			{
				cmdBuffer->Draw(frameIndex, 6);
			});
	}
}