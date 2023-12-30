#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/TransferPasses.h"
#include "Graphics/Rendering/Renderer.h"
#include "Graphics/DebugRender.h"

#include "Objects/Camera.h"
#include "Objects/Skybox.h"
#include "Objects/Light.h"
#include "Objects/Probe.h"
#include "Objects/Model.h"

#include "UI/UIContext.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace passes;
using namespace objects;

using namespace miru;
using namespace base;

void TransferPasses::Upload(Renderer& renderer, std::vector<Ref<Camera>>& allCameras, Ref<Skybox> skybox, std::vector<Ref<Light>>& lights, std::vector<Ref<Model>>& allQueue)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	Ref<TransferPassParameters> uploadPassParameters = CreateRef<TransferPassParameters>();

	//Cameras
	for (const auto& camera : allCameras)
	{
		if (camera && camera->GetUpdateGPUFlag())
		{
			uploadPassParameters->AddResourceView(camera->GetCameraUB());
			camera->ResetUpdateGPUFlag();
		}
	}

	//Skybox
	if (skybox && skybox->GetUpdateGPUFlag())
	{
		uploadPassParameters->AddResourceView(skybox->GetHDRTexture());
		skybox->ResetUpdateGPUFlag();
	}

	//Lights
	for (const auto& light : lights)
	{
		if (light)
		{
			if (light->GetUpdateGPUFlag())
			{
				uploadPassParameters->AddResourceView(light->GetUB());
				light->ResetUpdateGPUFlag();
			}

			//Probes
			const auto& probe = light->GetProbe();
			if (probe && probe->GetUpdateGPUFlag())
			{
				uploadPassParameters->AddResourceView(probe->GetUB());
				probe->ResetUpdateGPUFlag();
			}
		}
	}

	//Models
	for (const auto& model : allQueue)
	{
		if (model)
		{
			//Geometry
			if (model->GetUpdateGPUFlag())
			{
				for (const auto& vb : model->GetMesh()->GetVertexBuffers())
					uploadPassParameters->AddResourceView(vb);
				for (const auto& ib : model->GetMesh()->GetIndexBuffers())
					uploadPassParameters->AddResourceView(ib);

				uploadPassParameters->AddResourceView(model->GetUB());
				model->ResetUpdateGPUFlag();
			}

			//Materials
			for (const auto& material : model->GetMesh()->GetMaterials())
			{
				if (material && material->GetUpdateGPUFlag())
				{
					uploadPassParameters->AddResourceView(material->GetUB());
					for (const auto& texture : material->GetTextures())
						uploadPassParameters->AddResourceView(texture.second);

					material->ResetUpdateGPUFlag();
				}
			}
		}
	}

	//PostProcessing
	Renderer::PostProcessingInfo& postProcessingInfo = renderer.GetPostProcessingInfo();
	if (postProcessingInfo.bloom.UB)
	{
		postProcessingInfo.bloom.UB->SubmitData();
		uploadPassParameters->AddResourceView(postProcessingInfo.bloom.UB);
	}
	if (postProcessingInfo.hdrSettings.UB)
	{
		postProcessingInfo.hdrSettings.UB->SubmitData();
		uploadPassParameters->AddResourceView(postProcessingInfo.hdrSettings.UB);
	}

	//DebugRender
	Ref<Uniformbuffer<UniformBufferStructures::DebugProbeInfo>>& debugProbeInfoUB = DebugRender::GetDebugProbeInfo();
	if (debugProbeInfoUB)
	{
		debugProbeInfoUB->SubmitData();
		uploadPassParameters->AddResourceView(debugProbeInfoUB);
	}

	renderGraph.AddPass("Upload Transfer", uploadPassParameters, CommandPool::QueueType::TRANSFER, nullptr);
}