#pragma once
#include "gear_core_common.h"

namespace GEAR {
namespace GRAPHICS {
class Texture
{
private:
	void* m_Device;

	static miru::Ref<miru::crossplatform::Context> s_Context;
	static miru::Ref<miru::crossplatform::MemoryBlock> s_MB_CPU_Upload, s_MB_GPU_Usage;

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

	std::string m_Filepath;
	std::vector<std::string> m_CubemapFilepaths;
	unsigned char* m_LocalBuffer;
	int m_Width = 0, m_Height = 0, m_Depth = 0, m_BPP = 0; //BPP = Bits per pixel
	bool m_Cubemap = false;
	bool m_DepthTexture = false;
	float m_TileFactor = 1.0f;
	int m_Multisample = 1;
	float m_AnisotrophicValue;
	bool m_Upload = false;

public:
	//Simple Texture 2D.
	Texture(void* device, const std::string& filepath); 
	//For CubeMaps only! Submit in filepaths in order of: front, back, top, bottom, right and left.
	Texture(void* device, const std::vector<std::string>& filepaths);
	//For Fonts only!
	Texture(void* device, unsigned char* buffer, int width, int height);
	//General Texture constructor, suitable for compute shaders!
	Texture(void* device, unsigned char* buffer, miru::crossplatform::Image::Type type, miru::crossplatform::Image::Format format, int multisample, int width, int height, int depth);
	~Texture();

	inline static void SetContext(miru::Ref<miru::crossplatform::Context> context) { s_Context = context; };
	void InitialiseMemory();

	void GetInitialTransition(std::vector<miru::Ref<miru::crossplatform::Barrier>>& barriers);
	void Upload(miru::Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);
	void GetFinalTransition(std::vector<miru::Ref<miru::crossplatform::Barrier>>& barriers);

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline int GetBPP() const { return m_BPP; }

	inline miru::Ref<miru::crossplatform::Image>GetTexture() const { return m_Texture; }
	inline miru::Ref<miru::crossplatform::ImageView>GetTextureImageView() const { return m_TextureImageView; }
	inline miru::Ref<miru::crossplatform::Sampler>GetTextureSampler() const { return m_Sampler; }
	inline bool IsCubeMap() const { return m_Cubemap; }
	inline bool IsDepthTexture() const { return m_DepthTexture; }

	inline void SetTileFactor(float factor) { m_TileFactor = factor; CreateSampler();}
	inline float GetTileFactor() const { return m_TileFactor; }

	inline void GetAnisotrophicValue(float anisostrphicVal) { m_AnisotrophicValue = anisostrphicVal; CreateSampler();};
	inline float SetAnisotrophicValue() const { return m_AnisotrophicValue; };
	inline std::string GetAnisotrophicValue() { return std::to_string(static_cast<int>(m_AnisotrophicValue)); }

private:
	void AniostrophicFilting();
	void MipMapping();
	void CreateSampler();
};
}
}