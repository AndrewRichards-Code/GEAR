#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/TransferPasses.h"
#include "Graphics/Rendering/Renderer.h"

#include "Objects/Camera.h"
#include "Objects/Skybox.h"
#include "Objects/Light.h"
#include "Objects/Probe.h"
#include "Objects/Model.h"

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
	for (const auto& camera : allCameras)
	{
		if (camera && camera->GetUpdateGPUFlag())
		{
			uploadPassParameters->AddResourceView(camera->GetCameraUB());
			uploadPassParameters->AddResourceView(camera->GetHDRInfoUB());
			camera->ResetUpdateGPUFlag();
		}
	}
	if (skybox && skybox->GetUpdateGPUFlag())
	{
		uploadPassParameters->AddResourceView(skybox->GetHDRTexture());
		skybox->ResetUpdateGPUFlag();
	}
	for (const auto& light : lights)
	{
		if (light && light->GetUpdateGPUFlag())
		{
			uploadPassParameters->AddResourceView(light->GetUB());
			light->ResetUpdateGPUFlag();

			const auto& probe = light->GetProbe();
			if (probe && probe->GetUpdateGPUFlag())
			{
				uploadPassParameters->AddResourceView(probe->GetUB());
				probe->ResetUpdateGPUFlag();
			}
		}
	}
	for (const auto& model : allQueue)
	{
		if (model && model->GetUpdateGPUFlag())
		{
			for (const auto& vb : model->GetMesh()->GetVertexBuffers())
				uploadPassParameters->AddResourceView(vb);
			for (const auto& ib : model->GetMesh()->GetIndexBuffers())
				uploadPassParameters->AddResourceView(ib);

			uploadPassParameters->AddResourceView(model->GetUB());
			model->ResetUpdateGPUFlag();
		}
		if (model)
		{
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
	renderGraph.AddPass("Upload Transfer", uploadPassParameters, CommandPool::QueueType::TRANSFER, nullptr);
}