#pragma once

#include "gear_core_common.h"

namespace GEAR {
namespace GRAPHICS {

class Pipeline
{
private:
	void* m_Device;
	miru::Ref<miru::crossplatform::Pipeline> m_Pipeline;
	miru::crossplatform::Pipeline::CreateInfo m_PipelineCI;

	std::vector<miru::Ref<miru::crossplatform::DescriptorSetLayout>> m_DescSetLayouts;
	std::vector<miru::crossplatform::DescriptorSetLayout::CreateInfo> m_DescSetLayoutCIs;

	std::vector<miru::Ref<miru::crossplatform::Shader>> m_Shaders;

	miru::crossplatform::Pipeline::ViewportState m_ViewportState;
	miru::crossplatform::Pipeline::RasterisationState m_RasterisationState;
	miru::crossplatform::Pipeline::MultisampleState m_MultisampleState;
	miru::crossplatform::Pipeline::DepthStencilState m_DepthStencilState;
	miru::crossplatform::Pipeline::ColourBlendState m_ColourBlendState;
	miru::Ref<miru::crossplatform::RenderPass> m_RenderPass;
	uint32_t m_SubpassIndex;

public:
	Pipeline(void* device, const std::string& vertexPath, const std::string& fragmentPath);
	Pipeline(void* device, const std::string& computePath);
	~Pipeline();

	void AddAdditionalShaderModule(miru::crossplatform::Shader::StageBit stage, const std::string& shaderPath);
	void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
	void SetRasterisationState(bool depthClampEnable, bool rasteriserDiscardEnable, miru::crossplatform::PolygonMode polygonMode, miru::crossplatform::CullModeBit cullMode,
		miru::crossplatform::FrontFace frontFace, bool depthBiasEnable, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor, float lineWidth);
	void SetMultisampleState(miru::crossplatform::Image::SampleCountBit rasterisationSamples, bool sampleShadingEnable, float minSampleShading,
		bool alphaToCoverageEnable, bool alphaToOneEnable);
	void SetDepthStencilState(bool depthTestEnable, bool depthWriteEnable, miru::crossplatform::CompareOp depthCompareOp, bool depthBoundsTestEnable,
		bool stencilTestEnable, const miru::crossplatform::StencilOpState& front, const miru::crossplatform::StencilOpState& back, float minDepthBounds, float maxDepthBounds);
	void SetColourBlendState(bool logicOpEnable, miru::crossplatform::LogicOp logicOp, const std::vector<miru::crossplatform::ColourBlendAttachmentState> attachments, float blendConstant[4]);
	void SetRenderPass(miru::Ref<miru::crossplatform::RenderPass> renderPass, uint32_t subpassIndex) { m_RenderPass = renderPass; m_SubpassIndex = subpassIndex; };

	void FinalisePipline();

	inline miru::Ref<miru::crossplatform::Pipeline> GetPipeline() { return m_Pipeline; };
};
}
}
