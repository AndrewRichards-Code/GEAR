#pragma once
#include "gear_core_common.h"

namespace gear 
{
namespace graphics 
{
	class Texture;
	class RenderPipeline;

	class GenerateMipMaps
	{
	private:
		static gear::Ref<RenderPipeline> s_Pipeline;

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

		miru::Ref<miru::crossplatform::Fence> m_Fence;
		miru::crossplatform::Fence::CreateInfo m_FenceCI;

	public:
		GenerateMipMaps(Texture* texture);
		~GenerateMipMaps();

	};
}
}
