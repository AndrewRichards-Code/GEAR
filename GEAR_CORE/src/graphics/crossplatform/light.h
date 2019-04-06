#pragma once

#include "graphics/opengl/shader/shader.h"
#include "camera.h"
#include "maths/ARMLib.h"

#define GEAR_MAX_LIGHTS 8

namespace GEAR {
namespace GRAPHICS {
namespace CROSSPLATFORM {

#ifdef GEAR_OPENGL

class Light
{
public:
	enum class LightType : int
	{
		GEAR_LIGHT_POINT = 0,		//pos
		GEAR_LIGHT_DIRECTIONAL = 1,	//dir
		GEAR_LIGHT_SPOT = 2,		//cone, umbra, penumbra
		GEAR_LIGHT_AREA = 3			//err...
	};

private:
	static int s_NumOfLights;
	int m_LightID;
	LightType m_Type;
	OPENGL::Shader& m_Shader;

	static bool s_InitialiseUBO;
	struct LightUBO
	{
		ARM::Vec4 m_Colour;					//00
		ARM::Vec4 m_Position;				//16
		ARM::Vec4 m_Direction;				//32
		float m_Type;						//48
		float m_ShineFactor;				//52
		float m_Reflectivity;				//56
		float m_AmbientFactor;				//60
		float m_AttenuationConstant;		//64
		float m_AttenuationLinear;			//68
		float m_AttenuationQuadratic;		//72
		float m_CutOff;						//76
	}m_LightUBO;							//80 Total Size

	//Depth Shader for Shadows
	//const Shader m_DepthShader = Shader("res/shaders/GLSL/depth.vert", "res/shaders/GLSL/depth.frag");
	//Camera m_LightCam = Camera(GEAR_CAMERA_ORTHOGRAPHIC, m_DepthShader, m_PosDir, ARM::Vec3(0, 0, 1), ARM::Vec3(0, 1, 0));

public:
	ARM::Vec4 m_Colour;
	ARM::Vec3 m_Position;
	ARM::Vec3 m_Direction;

public:
	Light(LightType type, const ARM::Vec3& position, const ARM::Vec3& direction, const ARM::Vec4& colour, OPENGL::Shader& shader);
	~Light();

	void Specular(float shineFactor, float reflectivity);
	void Ambient(float ambientFactor);
	void Attenuation(float linear, float quadratic);
	void SpotCone(double theta);

	void Point();
	void Directional();
	void Spot();
	void Area();

	//inline const Shader& GetDepthShader() const { return m_DepthShader; }

	void CalculateLightMVP();
	void UpdateColour();
	void UpdatePosition();
	void UpdateDirection();
	void UpdateDirection(double yaw, double pitch, double roll, bool invertYAxis);

private:
	void InitialiseUBO();
	void SetAllToZero();
};
}
}
}
#endif