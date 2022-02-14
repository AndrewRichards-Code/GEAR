#pragma once

#include "gear_core_common.h"

namespace gear 
{
namespace graphics 
{
	class GEAR_API RenderPipeline
	{
	public:
		enum class ShaderBuildMode : uint32_t
		{
			NEVER,
			INCREMENTAL,
			ALWAYS
		};

		struct CreateInfo
		{
			std::string												debugName;
			std::vector<miru::crossplatform::Shader::CreateInfo>	shaderCreateInfo;
			miru::crossplatform::Pipeline::InputAssemblyState		inputAssemblyState;
			miru::crossplatform::Pipeline::ViewportState			viewportState;
			miru::crossplatform::Pipeline::RasterisationState		rasterisationState;
			miru::crossplatform::Pipeline::MultisampleState			multisampleState;
			miru::crossplatform::Pipeline::DepthStencilState		depthStencilState;
			miru::crossplatform::Pipeline::ColourBlendState			colourBlendState;
			Ref<miru::crossplatform::RenderPass>					renderPass;
			uint32_t												subpassIndex;
		};
		struct LoadInfo
		{
			void*										device;
			std::string									filepath;
			float										viewportWidth;
			float										viewportHeight;
			miru::crossplatform::Image::SampleCountBit	samples;
			Ref<miru::crossplatform::RenderPass>		renderPass;
			uint32_t									subpassIndex;
		};

	private:
		void* m_Device;
		Ref<miru::crossplatform::Pipeline> m_Pipeline;
		miru::crossplatform::Pipeline::CreateInfo m_PipelineCI;

		std::vector<Ref<miru::crossplatform::DescriptorSetLayout>> m_DescSetLayouts;
		std::vector<miru::crossplatform::DescriptorSetLayout::CreateInfo> m_DescSetLayoutCIs;

		std::vector<Ref<miru::crossplatform::Shader>> m_Shaders;
		std::vector<std::vector<miru::crossplatform::Shader::ResourceBindingDescription>> m_RBDs;

		static ShaderBuildMode s_ShaderBuildMode;

	public:
		CreateInfo m_CI;

	public:
		RenderPipeline(CreateInfo* pCreateInfo);
		RenderPipeline(LoadInfo* pLoadInfo);
		~RenderPipeline();

		const CreateInfo& GetCreateInfo() { return m_CI; }

		void FinalisePipline();
		void RecompileShaders();
		void Rebuild();

		inline static void SetShaderBuildMode(ShaderBuildMode buildMode) { s_ShaderBuildMode = buildMode; }

		inline const Ref<miru::crossplatform::Pipeline>& GetPipeline() const { return m_Pipeline; };
		inline const std::vector<Ref<miru::crossplatform::DescriptorSetLayout>>& GetDescriptorSetLayouts() { return m_DescSetLayouts; };
		inline const std::vector<std::vector<miru::crossplatform::Shader::ResourceBindingDescription>>& GetRBDs() { return m_RBDs; };
	};
}
}
