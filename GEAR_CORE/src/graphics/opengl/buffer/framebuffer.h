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
		std::array<std::shared_ptr<Texture>, 16> m_ColourTextures;
		std::shared_ptr<Texture> m_DepthTexture;

	public:
		FrameBuffer(int width, int height);
		~FrameBuffer();

		void Bind() const;
		void Unbind() const;

		void UpdateFrameBufferSize(int width, int height);

		void AddColourTextureAttachment(int attachment = 0);
		void UseColourTextureAttachment(int attachment = 0);

		inline std::shared_ptr<Texture> GetColourTexture(int slot = 0) const { return m_ColourTextures[slot]; }
		inline std::shared_ptr<Texture> GetDepthTexture() const { return m_DepthTexture; }

	private:
		void AddDepthTextureAttachment();
		void AddDepthBufferAttachment();
	};
}
}
}