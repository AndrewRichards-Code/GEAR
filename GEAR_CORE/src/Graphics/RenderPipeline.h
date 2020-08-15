#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
class RenderPipeline
{
public:
	struct CreateInfo
	{
		std::string												debugName;
		std::vector<miru::crossplatform::Shader::CreateInfo>	shaderCreateInfo;
		miru::crossplatform::Pipeline::ViewportState			viewportState;
		miru::crossplatform::Pipeline::RasterisationState		rasterisationState;
		miru::crossplatform::Pipeline::MultisampleState			multisampleState;
		miru::crossplatform::Pipeline::DepthStencilState		depthStencilState;
		miru::crossplatform::Pipeline::ColourBlendState			colourBlendState;
		miru::Ref<miru::crossplatform::RenderPass>				renderPass;
		uint32_t												subpassIndex;
	};

private:
	void* m_Device;
	miru::Ref<miru::crossplatform::Pipeline> m_Pipeline;
	miru::crossplatform::Pipeline::CreateInfo m_PipelineCI;

	std::vector<miru::Ref<miru::crossplatform::DescriptorSetLayout>> m_DescSetLayouts;
	std::vector<miru::crossplatform::DescriptorSetLayout::CreateInfo> m_DescSetLayoutCIs;

	std::vector<miru::Ref<miru::crossplatform::Shader>> m_Shaders;
	
public:
	CreateInfo m_CI;

public:
	RenderPipeline(CreateInfo* pCreateInfo);
	~RenderPipeline();

	void FinalisePipline();
	void RecompileShaders();
	void Rebuild();

	inline const miru::Ref<miru::crossplatform::Pipeline>& GetPipeline() const { return m_Pipeline; };
	inline const std::vector<miru::Ref<miru::crossplatform::DescriptorSetLayout>>& GetDescriptorSetLayouts() { return m_DescSetLayouts; };
};
}
}
