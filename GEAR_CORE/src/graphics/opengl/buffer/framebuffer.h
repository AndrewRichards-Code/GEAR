#pragma once

#include "gear_common.h"
#include "graphics/opengl/texture.h"

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
	class FrameBuffer
	{
	private:
		unsigned int m_FrameID;
		unsigned int m_RenderBufferID;
		int m_Width, m_Height;
		bool m_CubeMap; 
		int m_Multisample;
		Texture::ImageFormat m_Format;
		std::array<std::shared_ptr<Texture>, 16> m_ColourTextures;
		std::shared_ptr<Texture> m_DepthTexture;

		std::unique_ptr<FrameBuffer> m_ResolvedFBO = nullptr;

	public:
		FrameBuffer(int width, int height, int multisample = 1, bool cubeMap = false, Texture::ImageFormat format = Texture::ImageFormat::GEAR_RGBA8);
		~FrameBuffer();

		void Bind() const;
		void Unbind() const;
		void BindResolved() const;
		void UnbindResolved() const;

		void UpdateFrameBufferSize(int width, int height);

		void AddColourTextureAttachment(int attachment = 0);
		void DrawToColourTextureAttachment(int attachment = 0);
		void ReadFromColourTextureAttachment(int attachment = 0);

		inline void Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

		void Resolve();

		inline std::shared_ptr<Texture> GetColourTexture(int slot = 0, bool getResolvedFBO = true) const 
		{
			if(m_ResolvedFBO && getResolvedFBO)
				return m_ResolvedFBO->m_ColourTextures[slot];
			else
				return m_ColourTextures[slot]; 
		}
		inline std::shared_ptr<Texture> GetDepthTexture(bool getResolvedFBO = true) const
		{
			if(m_ResolvedFBO && getResolvedFBO)
				return m_ResolvedFBO->m_DepthTexture;
			else
				return m_DepthTexture; 
		}

		inline int GetMultisampleValue() const { return m_Multisample; }

	private:
		void AddDepthTextureAttachment();
		void AddDepthBufferAttachment();
		void CheckColourTextureAttachments(int attachment);
	};
}
}
}