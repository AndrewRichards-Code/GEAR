#pragma once

#include "gear_core_common.h"
#include "graphics/texture.h"

namespace gear {
namespace graphics {
	class FrameBuffer
	{
	public:
		struct CreateInfo
		{
			void*										device;
			uint32_t									width;
			uint32_t									height;
			miru::crossplatform::Image::SampleCountBit	samples;
			miru::Ref<miru::crossplatform::RenderPass>	renderPass;
			bool										cubemap;
		};

	private:
		miru::Ref<miru::crossplatform::Framebuffer> m_Framebuffer;
		miru::crossplatform::Framebuffer::CreateInfo m_FramebufferCI;

		std::array<gear::Ref<Texture>, 8> m_ColourTextures;
		gear::Ref<Texture> m_DepthTexture;

		CreateInfo m_CI;

	public:
		FrameBuffer(CreateInfo* pCreateInfo);
		~FrameBuffer();

		void UpdateFrameBufferSize(uint32_t width, uint32_t height);

		void AddColourTextureAttachment(size_t attachment = 0);
		void AddDepthTextureAttachment(size_t attachment = 0);

		void FinaliseFramebuffer();

		inline gear::Ref<Texture> GetColourTexture(size_t slot = 0, bool getResolvedFBO = true) const
		{
			return m_ColourTextures[slot]; 
		}
		inline gear::Ref<Texture> GetDepthTexture(bool getResolvedFBO = true) const
		{
			return m_DepthTexture; 
		}
		inline miru::crossplatform::Image::SampleCountBit GetMultisampleValue() const { return m_CI.samples; }
		inline miru::Ref<miru::crossplatform::Framebuffer> GetFramebuffer() { return m_Framebuffer; }

	private:
		void CheckColourTextureAttachments(size_t attachment);
	};
}
}