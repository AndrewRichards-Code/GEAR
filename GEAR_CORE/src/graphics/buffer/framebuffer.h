#pragma once

#include <memory>
#include <array>
#include "GL/glew.h"
#include "../texture.h"
#include "../window.h"

namespace GEAR {
namespace GRAPHICS {
class FrameBuffer
{
private:
	const Window& m_Window;
	unsigned int m_FrameID;
	unsigned int m_RenderBufferID;
	std::array<std::shared_ptr<Texture>, 16> m_ColourTextures;
	std::shared_ptr<Texture> m_DepthTexture;

public:
	FrameBuffer(const Window& window);
	~FrameBuffer();

	void Bind() const;
	void Unbind() const;

	void UpdateFrameBufferSize();
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