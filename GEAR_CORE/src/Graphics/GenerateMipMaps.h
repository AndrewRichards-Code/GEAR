#pragma once
#include "gear_core_common.h"

#include "Graphics/RenderPipeline.h"
#include "Graphics/Texture.h"

namespace gear 
{
namespace graphics 
{
	class GenerateMipMaps
	{
	private:
		static gear::Ref<RenderPipeline> s_GenerateMipMapPipeline;
		static RenderPipeline::LoadInfo s_GenerateMipMapPipelineLI;

		miru::Ref<miru::crossplatform::CommandPool> m_ComputeCmdPool;
		miru::crossplatform::CommandPool::CreateInfo m_ComputeCmdPoolCI;

		miru::Ref<miru::crossplatform::CommandBuffer> m_ComputeCmdBuffer;
		miru::crossplatform::CommandBuffer::CreateInfo m_ComputeCmdBufferCI;

		std::vector<miru::Ref<miru::crossplatform::ImageView>> m_ImageViews;
		miru::crossplatform::ImageView::CreateInfo m_ImageViewCI;

		miru::Ref<miru::crossplatform::Sampler> m_Sampler;
		miru::crossplatform::Sampler::CreateInfo m_SamplerCI;

		miru::Ref<miru::crossplatform::DescriptorPool> m_DescPool;
		miru::crossplatform::DescriptorPool::CreateInfo m_DescPoolCI;

		std::vector<miru::Ref<miru::crossplatform::DescriptorSet>> m_DescSets;
		miru::crossplatform::DescriptorSet::CreateInfo m_DescSetCI;

	public:
		GenerateMipMaps(gear::Ref<Texture>& texture);
		~GenerateMipMaps();

	};
}
}
