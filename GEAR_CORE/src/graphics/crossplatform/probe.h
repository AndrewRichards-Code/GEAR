#pragma once

#include "gear_common.h"
#include "maths/ARMLib.h"
#include "graphics/opengl/buffer/framebuffer.h"
#include "camera.h"
#include "object.h"

namespace GEAR {
namespace GRAPHICS {
namespace CROSSPLATFORM {

#ifdef GEAR_OPENGL

class Probe
{
private:
	const ARM::Vec3 m_Position;
	int m_Width, m_Height;
	int m_Multisample;
	OPENGL::Texture::ImageFormat m_Format;

	std::shared_ptr<OPENGL::FrameBuffer> m_FrameBuffer;
	std::unique_ptr<Camera> m_Camera;
	bool m_IsResolved = false;

	std::shared_ptr<OPENGL::Texture> m_Cubemap = std::make_shared<OPENGL::Texture>(OPENGL::Texture::TextureType::GEAR_TEXTURE_CUBE_MAP, m_Format, 1, m_Width, m_Height);

	const ARM::Vec3 m_X = ARM::Vec3(1, 0, 0);
	const ARM::Vec3 m_Y = ARM::Vec3(0, 1, 0);
	const ARM::Vec3 m_Z = ARM::Vec3(0, 0, 1);
	ARM::Mat4 m_ViewMatrices[6] = 
	{
		ARM::Quat(static_cast<float>(+ARM::pi / 2), m_Y).ToMat4(),	//+X
		ARM::Quat(static_cast<float>(-ARM::pi / 2), m_Y).ToMat4(),	//-X
		ARM::Quat(static_cast<float>(-ARM::pi / 2), m_X).ToMat4(),	//+Y
		ARM::Quat(static_cast<float>(+ARM::pi / 2), m_X).ToMat4(),	//-Y
		ARM::Quat(static_cast<float>(+ARM::pi	 ), m_Y).ToMat4(),	//-Z
		ARM::Quat(static_cast<float>(0			 ), m_Y).ToMat4(),	//+Z
	};

public:
	Probe(const ARM::Vec3& position, int width, int height, int multisample = 1, OPENGL::Texture::ImageFormat format = OPENGL::Texture::ImageFormat::GEAR_RGBA8);
	~Probe();

	std::shared_ptr<OPENGL::Texture> ConstructCubemap();
	void Resolve();
	void Render(const std::deque<Object*> renderQueue, int windowWidth, int windowHeight);

	inline std::shared_ptr<OPENGL::FrameBuffer> GetFrameBuffer() { return m_FrameBuffer; }
};

#endif
}
}
}