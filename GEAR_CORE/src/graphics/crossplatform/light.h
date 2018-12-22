#pragma once

#include "../opengl/shader.h"
#include "camera.h"
#include "../../maths/ARMLib.h"

#define GEAR_LIGHT_POINT 0			//pos
#define GEAR_LIGHT_DIRECTIONAL 1	//dir
#define GEAR_LIGHT_SPOT 2			//cone, umbra, penumbra
#define GEAR_LIGHT_AREA 3			//err...

#define GEAR_MAX_LIGHTS 8

namespace GEAR {
namespace GRAPHICS {
namespace CROSSPLATFORM {

#ifdef GEAR_OPENGL

class Light
{
private:
	static int s_NumOfLights;
	int m_LightID;
	int m_Type;
	const OPENGL::Shader& m_Shader;

	//Depth Shader for Shadows
	//const Shader m_DepthShader = Shader("res/shaders/depth.vert", "res/shaders/depth.frag");
	//Camera m_LightCam = Camera(GEAR_CAMERA_ORTHOGRAPHIC, m_DepthShader, m_PosDir, ARM::Vec3(0, 0, 1), ARM::Vec3(0, 1, 0));

public:
	ARM::Vec4 m_Colour;
	ARM::Vec3 m_Position;
	ARM::Vec3 m_Direction;

public:
	Light(int type, const ARM::Vec3& position, ARM::Vec3& direction, const ARM::Vec4& colour, const OPENGL::Shader& shader);
	~Light();

	void Specular(float shineFactor, float reflectivity) const;
	void Ambient(float ambientFactor) const;
	void Attenuation(float linear, float quadratic) const;
	void SpotCone(double theta) const;

	void Point() const;
	void Directional() const;
	void Spot() const;
	void Area() const;

	//inline const Shader& GetDepthShader() const { return m_DepthShader; }

	void CalculateLightMVP();
	void UpdateColour();
	void UpdatePosition();
	void UpdateDirection(double yaw, double pitch, double roll, bool invertYAxis);
};
}
}
}
#endif