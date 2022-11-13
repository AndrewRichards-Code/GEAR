#include "gear_core_common.h"

#include "Graphics/Rendering/Passes/MipmapPasses.h"
#include "Graphics/Rendering/Renderer.h"

using namespace gear;
using namespace graphics;
using namespace rendering;
using namespace passes;

using namespace miru;
using namespace base;

void MipmapPasses::GenerateMipmaps(Renderer& renderer, Ref<Texture> texture)
{
	RenderGraph& renderGraph = renderer.GetRenderGraph();
	const uint32_t& levels = texture->GetCreateInfo().mipLevels;
	const uint32_t& layers = texture->GetCreateInfo().arrayLayers;
	const std::string& name = texture->GetCreateInfo().debugName;
	std::vector<Ref<ImageView>> mipImageViews;

	for (uint32_t i = 0; i < levels; i++)
		mipImageViews.push_back(renderGraph.CreateImageView(texture->GetImage(), layers > 1 ? Image::Type::TYPE_2D_ARRAY : Image::Type::TYPE_2D, { Image::AspectBit::COLOUR_BIT, i, 1, 0, layers }));

	Ref<TaskPassParameters> generateMipMapsPassParameters;
	for (uint32_t i = 0; i < (levels - 1); i++)
	{
		if (layers > 1)
		{
			generateMipMapsPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["MipmapArray"]);
			generateMipMapsPassParameters->SetResourceView("inputImageArray", ResourceView(mipImageViews[std::min(i + 0, levels)], Resource::State::SHADER_READ_ONLY));
			generateMipMapsPassParameters->SetResourceView("outputImageArray", ResourceView(mipImageViews[std::min(i + 1, levels)], Resource::State::SHADER_READ_WRITE));
		}
		else
		{
			generateMipMapsPassParameters = CreateRef<TaskPassParameters>(renderer.GetRenderPipelines()["Mipmap"]);
			generateMipMapsPassParameters->SetResourceView("inputImage", ResourceView(mipImageViews[std::min(i + 0, levels)], Resource::State::SHADER_READ_ONLY));
			generateMipMapsPassParameters->SetResourceView("outputImage", ResourceView(mipImageViews[std::min(i + 1, levels)], Resource::State::SHADER_READ_WRITE));
		}
		const std::array<uint32_t, 3>& groupCount = generateMipMapsPassParameters->GetPipeline()->GetCreateInfo().shaders[0]->GetGroupCountXYZ();

		i++;
		renderGraph.AddPass(name + " - Level: " + std::to_string(i), generateMipMapsPassParameters, CommandPool::QueueType::COMPUTE,
			[i, layers, texture, groupCount](Ref<CommandBuffer>& cmdBuffer, uint32_t frameIndex)
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