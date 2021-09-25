#pragma once
#include "gear_core_common.h"
#include "ImageProcessing.h"

namespace gear
{
namespace graphics
{
	class PostProcessing
	{
	public:
		struct ImageResourceInfo
		{
			Ref<miru::crossplatform::Image>			image;
			miru::crossplatform::Barrier::AccessBit srcAccess;
			miru::crossplatform::Image::Layout		oldLayout;
			miru::crossplatform::PipelineStageBit	srcStage;
		};

	private:
		static Ref<RenderPipeline> s_BloomPreFilter;
		static Ref<RenderPipeline> s_BloomDownsample;
		static Ref<RenderPipeline> s_BloomUpsample;

	public: 
		PostProcessing();
		~PostProcessing();

		static void Bloom(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const ImageResourceInfo& IRI);
	
	private:
		static void BloomPreFilter(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const ImageResourceInfo& IRI);
		static void BloomDownsample(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const ImageResourceInfo& IRI);
		static void BloomUpsample(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const ImageResourceInfo& IRI);

	public:
		static void SaveImageViewsAndDescriptorSets(const std::vector<Ref<miru::crossplatform::ImageView>>& imageViews, const std::vector<Ref<miru::crossplatform::DescriptorSet>>& descSets, size_t idx)
		{
			for (const auto& imageView : imageViews)
				s_ImageViews[idx].push_back(imageView);
			for (const auto& descSet : descSets)
				s_DescSets[idx].push_back(descSet);
		}
		static void ClearImageViewsAndDescriptorSets(size_t idx)
		{
			s_ImageViews[idx].clear();
			s_DescSets[idx].clear();
		}
		static std::array<std::vector<Ref<miru::crossplatform::ImageView>>, 2> s_ImageViews;
		static std::array<std::vector<Ref<miru::crossplatform::DescriptorSet>>, 2> s_DescSets;
	};
}
}
