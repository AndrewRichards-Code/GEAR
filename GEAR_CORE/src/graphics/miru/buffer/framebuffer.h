#pragma once

#include "gear_core_common.h"
#include "graphics/miru/texture.h"

namespace GEAR {
namespace GRAPHICS {
	class FrameBuffer
	{
	private:
		void* m_Device;

		miru::Ref<miru::crossplatform::Framebuffer> m_Framebuffer;
		miru::crossplatform::Framebuffer::CreateInfo m_FramebufferCI;

		int m_Width, m_Height;
		bool m_CubeMap; 
		int m_Multisample;
		std::array<std::shared_ptr<Texture>, 8> m_ColourTextures;
		std::shared_ptr<Texture> m_DepthTexture;


	public:
		FrameBuffer(void* device, int width, int height, int multisample = 1, bool cubeMap = false);
		~FrameBuffer();

		void UpdateFrameBufferSize(int width, int height);

		void AddColourTextureAttachment(int attachment = 0);
		void AddDepthTextureAttachment(int attachment = 0);

		void FinaliseFramebuffer(miru::Ref<miru::crossplatform::RenderPass> renderPass);

		inline std::shared_ptr<Texture> GetColourTexture(int slot = 0, bool getResolvedFBO = true) const 
		{
			return m_ColourTextures[slot]; 
		}
		inline std::shared_ptr<Texture> GetDepthTexture(bool getResolvedFBO = true) const
		{
			return m_DepthTexture; 
		}
		inline int GetMultisampleValue() const { return m_Multisample; }
		inline miru::Ref<miru::crossplatform::Framebuffer> GetFramebuffer() { return m_Framebuffer; }

	private:
		void CheckColourTextureAttachments(int attachment);
	};
}
}