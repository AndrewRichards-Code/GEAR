#pragma once

#include "gear_common.h"
#include "mars.h"
#include "graphics/opengl/buffer/framebuffer.h"
#include "graphics/opengl/renderer/batchrenderer3d.h"
#include "camera.h"
#include "object.h"

namespace GEAR {
namespace GRAPHICS {
namespace CROSSPLATFORM {

#ifdef GEAR_OPENGL

class OmniProbe
{
private:
	const mars::Vec3 m_Position;
	int m_Size;
	int m_Multisample;
	OPENGL::Texture::ImageFormat m_Format;

	std::shared_ptr<OPENGL::FrameBuffer> m_FrameBuffer;
	std::shared_ptr<Camera> m_Camera;

	std::shared_ptr<OPENGL::Texture> m_Cubemap = std::make_shared<OPENGL::Texture>(OPENGL::Texture::TextureType::GEAR_TEXTURE_CUBE_MAP, m_Format, 1, m_Size, m_Size);

	const mars::Vec3 m_X = mars::Vec3(1, 0, 0);
	const mars::Vec3 m_Y = mars::Vec3(0, 1, 0);
	const mars::Vec3 m_Z = mars::Vec3(0, 0, 1);
	mars::Mat4 m_ViewMatrices[6] = 
	{
		mars::Quat(static_cast<float>(+mars::pi / 2), m_Y).ToMat4(),	//+X
		mars::Quat(static_cast<float>(-mars::pi / 2), m_Y).ToMat4(),	//-X
		mars::Quat(static_cast<float>(-mars::pi / 2), m_X).ToMat4(),	//+Y
		mars::Quat(static_cast<float>(+mars::pi / 2), m_X).ToMat4(),	//-Y
		mars::Quat(static_cast<float>(+mars::pi	 ), m_Y).ToMat4(),	//-Z
		mars::Quat(static_cast<float>(0			 ), m_Y).ToMat4(),	//+Z
	};

	OPENGL::BatchRenderer3D r;

public:
	OmniProbe(const mars::Vec3& position, int size, int multisample = 1, OPENGL::Texture::ImageFormat format = OPENGL::Texture::ImageFormat::GEAR_RGBA8);
	~OmniProbe();

	void Resolve();
	void Render(const std::deque<Object*>& renderQueue, int windowWidth, int windowHeight, const OPENGL::Shader* overrideShader = nullptr);
	void UpdatePosition(const mars::Vec3& position);

	inline std::shared_ptr<OPENGL::Texture> GetCubemap() { return m_Cubemap; }
	inline std::shared_ptr<OPENGL::FrameBuffer> GetFrameBuffer() { return m_FrameBuffer; }
};

class UniProbe
{
private:
	const mars::Vec3 m_Position;
	const mars::Vec3 m_Direction;
	int m_Size;
	int m_Multisample;
	OPENGL::Texture::ImageFormat m_Format;

	std::shared_ptr<OPENGL::FrameBuffer> m_FrameBuffer;
	std::shared_ptr<Camera> m_Camera;

	std::shared_ptr<OPENGL::Texture> m_Texture = std::make_shared<OPENGL::Texture>(OPENGL::Texture::TextureType::GEAR_TEXTURE_2D, m_Format, 1, m_Size, m_Size);

	OPENGL::BatchRenderer3D r;

public:
	UniProbe(const mars::Vec3& position, const mars::Vec3& direction, int size, int projectionType = GEAR_CAMERA_PERSPECTIVE, int multisample = 1, OPENGL::Texture::ImageFormat format = OPENGL::Texture::ImageFormat::GEAR_RGBA8);
	~UniProbe();

	void Resolve();
	void Render(const std::deque<Object*>& renderQueue, int windowWidth, int windowHeight, const OPENGL::Shader* overrideShader = nullptr);
	void UpdatePosition(const mars::Vec3& position);

	inline std::shared_ptr<OPENGL::Texture> GetTexture() { return m_Texture; }
	inline std::shared_ptr<OPENGL::FrameBuffer> GetFrameBuffer() { return m_FrameBuffer; }
};

#endif
}
}
}