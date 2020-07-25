#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
class Texture
{
public:
	//Provide either filepaths or data, size and image dimension details.
	struct CreateInfo
	{
		const char*									debugName;
		void*										device;
		std::vector<std::string>					filepaths;	//Option 1
		const uint8_t*								data;		//Option 2
		size_t										size;		//Option 2
		uint32_t									width;		//Option 2
		uint32_t									height;		//Option 2
		uint32_t									depth;		//Option 2
		miru::crossplatform::Image::Format			format;
		miru::crossplatform::Image::Type			type;
		miru::crossplatform::Image::SampleCountBit	samples;
	};

private:
	miru::Ref<miru::crossplatform::Image> m_Texture;
	miru::crossplatform::Image::CreateInfo m_TextureCI;
	miru::Ref<miru::crossplatform::Buffer> m_TextureUploadBuffer;
	miru::crossplatform::Buffer::CreateInfo m_TextureUploadBufferCI;

	miru::Ref<miru::crossplatform::Barrier> m_InitialBarrier, m_FinalBarrier;
	miru::crossplatform::Barrier::CreateInfo m_InitialBarrierCI, m_FinalBarrierCI;

	miru::Ref<miru::crossplatform::ImageView> m_TextureImageView;
	miru::crossplatform::ImageView::CreateInfo m_TextureImageViewCI;

	miru::Ref<miru::crossplatform::Sampler> m_Sampler;
	miru::crossplatform::Sampler::CreateInfo m_SamplerCI;

	std::string m_DebugName_TexUpload;
	std::string m_DebugName_Tex;
	std::string m_DebugName_TexIV;
	std::string m_DebugName_Sampler;

	CreateInfo m_CI;
	
	uint8_t* m_LocalBuffer;
	int m_BPP = 0; //BPP = Bits per pixel
	bool m_Cubemap = false;
	bool m_DepthTexture = false;
	
	float m_TileFactor = 1.0f;
	float m_AnisotrophicValue = 1.0f;
	
	bool m_Upload = false;
	bool m_InitialTransition = false;
	bool m_FinalTransition = false;

public:
	Texture(CreateInfo* pCreateInfo);
	~Texture();

	void GetInitialTransition(std::vector<miru::Ref<miru::crossplatform::Barrier>>& barriers, bool force = false);
	void Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);
	void GetFinalTransition(std::vector<miru::Ref<miru::crossplatform::Barrier>>& barriers, bool force = false);

	inline int GetWidth() const { return m_CI.width; }
	inline int GetHeight() const { return m_CI.height; }
	inline int GetBPP() const { return m_BPP; }

	inline const miru::Ref<miru::crossplatform::Image>& GetTexture() const { return m_Texture; }
	inline const miru::Ref<miru::crossplatform::ImageView>& GetTextureImageView() const { return m_TextureImageView; }
	inline const miru::Ref<miru::crossplatform::Sampler>& GetTextureSampler() const { return m_Sampler; }
	inline bool IsCubeMap() const { return m_Cubemap; }
	inline bool IsDepthTexture() const { return m_DepthTexture; }

	inline void SetTileFactor(float factor) { m_TileFactor = factor; CreateSampler();}
	inline float GetTileFactor() const { return m_TileFactor; }

	inline void SetAnisotrophicValue(float anisostrphicVal) { m_AnisotrophicValue = anisostrphicVal; CreateSampler();};
	inline float GetAnisotrophicValue() const { return m_AnisotrophicValue; };
	inline std::string GetAnisotrophicValue() { return std::to_string(static_cast<int>(m_AnisotrophicValue)); }

private:
	void AniostrophicFilting();
	void MipMapping();
	void CreateSampler();
};
}
}