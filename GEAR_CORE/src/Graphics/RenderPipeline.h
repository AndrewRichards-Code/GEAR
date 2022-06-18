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
				std::string											debugName;
				std::vector<miru::base::Shader::CreateInfo>			shaderCreateInfo;
				miru::base::Pipeline::InputAssemblyState			inputAssemblyState;
				miru::base::Pipeline::ViewportState					viewportState;
				miru::base::Pipeline::RasterisationState			rasterisationState;
				miru::base::Pipeline::MultisampleState				multisampleState;
				miru::base::Pipeline::DepthStencilState				depthStencilState;
				miru::base::Pipeline::ColourBlendState				colourBlendState;
				miru::base::Pipeline::DynamicRenderingCreateInfo	dynamicRendering;
			};
			struct LoadInfo
			{
				void* device;
				std::string								filepath;
				miru::base::Image::SampleCountBit		samples;
				std::vector<miru::base::Image::Format>	colourAttachmentFormats;
				miru::base::Image::Format				depthAttachmentFormat;
			};
			typedef std::vector<std::vector<miru::base::Shader::ResourceBindingDescription>> ResourceBindingDescriptions;

		private:
			void* m_Device;
			miru::base::PipelineRef m_Pipeline;
			miru::base::Pipeline::CreateInfo m_PipelineCI;

			std::vector<miru::base::DescriptorSetLayoutRef> m_DescSetLayouts;
			std::vector<miru::base::DescriptorSetLayout::CreateInfo> m_DescSetLayoutCIs;

			std::vector<miru::base::ShaderRef> m_Shaders;
			ResourceBindingDescriptions m_RBDs;

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

			inline const miru::base::PipelineRef& GetPipeline() const { return m_Pipeline; };
			inline const std::vector<miru::base::DescriptorSetLayoutRef>& GetDescriptorSetLayouts() { return m_DescSetLayouts; };
			inline const ResourceBindingDescriptions& GetRBDs() { return m_RBDs; };
		};
	}
}
