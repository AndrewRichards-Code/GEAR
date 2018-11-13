#pragma once

#include <memory>
#include "GL/glew.h"
#include "../texture.h"
#include "../object.h"
#include "../window.h"

namespace GEAR {
namespace GRAPHICS {
class FrameBuffer
{
private:
	const Window& m_Window;
	const Shader& m_Shader;
	unsigned int m_FrameID;
	unsigned int m_RenderBufferID;
	std::unique_ptr<Texture> m_DepthTexture;
	std::unique_ptr<Object> m_Quad;

public:
	FrameBuffer(const Window& window, const Shader& shader);
	~FrameBuffer();

	void Bind() const;
	void Unbind() const;

	Object UseFrameBufferAsObject(const ARM::Vec3 & translate, const ARM::Vec3 & scale);
	void UpdateFrameBufferSize();

private:
	void AttachDepthTexture();
	void AddDepthBuffer();
};
}
}