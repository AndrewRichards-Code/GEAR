#pragma once

#include "shader.h"
#include "camera.h"
#include "../maths/ARMLib.h"

#define GEAR_LIGHT_POINT 0			//pos
#define GEAR_LIGHT_DIRECTIONAL 1	//dir
#define GEAR_LIGHT_SPOT 2			//cone, umbra, penumbra
#define GEAR_LIGHT_AREA 3			//err...

namespace GEAR {
namespace GRAPHICS {

class Light
{
private:
	int m_Type;
	ARM::Vec4 m_Colour;
	const Shader& m_Shader;
	const Shader m_DepthShader = Shader("res/shaders/depth.vert", "res/shaders/depth.frag");
	Camera m_LightCam = Camera(GEAR_CAMERA_ORTHOGRAPHIC, m_DepthShader, m_PosDir, ARM::Vec3(0, 0, 1), ARM::Vec3(0, 1, 0));

public:
	ARM::Vec3 m_PosDir;

public:
	Light(int type, const ARM::Vec3& pos_dir, const ARM::Vec4& colour, const Shader& shader);
	~Light();

	void Specular(float shineFactor, float reflectivity) const;
	void Ambient(float ambientFactor) const;

	void Point() const;
	void Directional() const;
	void Spot() const;
	void Area() const;

	inline const Shader& GetDepthShader() const { return m_DepthShader; }

	void CalculateLightMVP();
};
}
}

