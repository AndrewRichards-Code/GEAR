#pragma once

#include "gear_core_common.h"

namespace gear 
{
namespace graphics 
{
	class Texture
	{
	public:
		//Provide either filepaths or data, size and image dimension details.
		struct CreateInfo
		{
			std::string									debugName;
			void*										device;
			std::vector<std::string>					filepaths;		//Option 1
			const uint8_t*								data;			//Option 2
			size_t										size;			//Option 2
			uint32_t									width;			//Option 2
			uint32_t									height;			//Option 2
			uint32_t									depth;			//Option 2
			uint32_t									mipLevels;
			miru::crossplatform::Image::Type			type;
			miru::crossplatform::Image::Format			format;
			miru::crossplatform::Image::SampleCountBit	samples;
			miru::crossplatform::Image::UsageBit		usage;
			bool										generateMipMaps;
		};

		struct SubresouresTransitionInfo
		{
			miru::crossplatform::Barrier::AccessBit			srcAccess;
			miru::crossplatform::Barrier::AccessBit			dstAccess;
			miru::crossplatform::Image::Layout				oldLayout;
			miru::crossplatform::Image::Layout				newLayout;
			miru::crossplatform::Image::SubresourceRange	subresoureRange; //Option 1
			bool											allSubresources; //Option 2
		};

	private:
		miru::Ref<miru::crossplatform::Image> m_Texture;
		miru::crossplatform::Image::CreateInfo m_TextureCI;
		miru::Ref<miru::crossplatform::Buffer> m_TextureUploadBuffer;
		miru::crossplatform::Buffer::CreateInfo m_TextureUploadBufferCI;

		miru::Ref<miru::crossplatform::ImageView> m_TextureImageView;
		miru::crossplatform::ImageView::CreateInfo m_TextureImageViewCI;

		miru::Ref<miru::crossplatform::Sampler> m_Sampler;
		miru::crossplatform::Sampler::CreateInfo m_SamplerCI;

		CreateInfo m_CI;

		int m_BPP = 0; //BPP = Bits per pixel
		bool m_HDR = false;
		bool m_Cubemap = false;
		bool m_DepthTexture = false;

		bool m_Upload = false;
		bool m_GenerateMipMaps = false;

		float m_AnisotrophicValue = 1.0f;

		typedef std::map<uint32_t, std::map<uint32_t, miru::crossplatform::Image::Layout>> SubresourceMap;
		SubresourceMap m_SubresourceMap; //m_SubResourceMap[mipLevel][arrayLayer] = { currentLayout };

	public:
		bool m_TransitionUnknownToTransferDst = false;
		bool m_TransitionTransferDstToShaderReadOnly = false;

	public:
		Texture(CreateInfo* pCreateInfo);
		~Texture();

		const CreateInfo& GetCreateInfo() { return m_CI; }

		void Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);
		void TransitionSubResources(std::vector<miru::Ref<miru::crossplatform::Barrier>>& barriers, const std::vector<SubresouresTransitionInfo>& transitionInfos);
		void GenerateMipMaps();
		void Reload();

		inline int GetWidth() const { return m_CI.width; }
		inline int GetHeight() const { return m_CI.height; }
		inline int GetBPP() const { return m_BPP; }

		inline const miru::Ref<miru::crossplatform::Image>& GetTexture() const { return m_Texture; }
		inline const miru::Ref<miru::crossplatform::ImageView>& GetTextureImageView() const { return m_TextureImageView; }
		inline const miru::Ref<miru::crossplatform::Sampler>& GetTextureSampler() const { return m_Sampler; }

		inline bool IsCubemap() const { return m_Cubemap; }
		inline bool IsDepthTexture() const { return m_DepthTexture; }
		inline const CreateInfo& GetCreateInfo() const { return m_CI; }

		inline void SetAnisotrophicValue(float anisostrphicVal) { m_AnisotrophicValue = anisostrphicVal; CreateSampler(); };
		inline float GetAnisotrophicValue() const { return m_AnisotrophicValue; };
		inline std::string GetAnisotrophicValue() { return std::to_string(static_cast<int>(m_AnisotrophicValue)); }

	private:
		void AniostrophicFilting();
		void MipMapping();
		void CreateSampler();

		//This populates the parameter imageData.
		void LoadImageData(std::vector<uint8_t>& imageData);
	};
}
}