#pragma once

//#include "graphics/opengl/shader/shader.h"
#include "camera.h"
//#include "probe.h"
#include "mars.h"

#define GEAR_MAX_LIGHTS 8

namespace GEAR {
namespace OBJECTS {

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
	void* m_Device;

	static int s_NumOfLights;
	int m_LightID;
	LightType m_Type;

	static bool s_InitialiseUBO;
	struct LightUBO
	{
		mars::Vec4 m_Colour;				//00
		mars::Vec4 m_Position;				//16
		mars::Vec4 m_Direction;				//32
		float m_Type;						//48
		float m_ShineFactor;				//52
		float m_Reflectivity;				//56
		float m_AmbientFactor;				//60
		float m_AttenuationConstant;		//64
		float m_AttenuationLinear;			//68
		float m_AttenuationQuadratic;		//72
		float m_CutOff;						//76
	}m_LightUBO;							//80 Total Size
	struct LightingUBO
	{
		float u_Diffuse;
		float u_Specular;
		float u_Ambient;
		float u_Emit;
	}m_LightingUBO;
	std::shared_ptr<GRAPHICS::UniformBuffer> m_UBO0;
	std::shared_ptr<GRAPHICS::UniformBuffer> m_UBO1;
	
	//Depth Shader for Shadows
	/*const OPENGL::Shader m_DepthShader = OPENGL::Shader("res/shaders/GLSL/depth.vert", "res/shaders/GLSL/depth.frag");
	const int m_DepthRenderSize = 128;
	std::unique_ptr<OmniProbe> m_DepthRenderTargetOmniProbe; 
	std::unique_ptr<UniProbe> m_DepthRenderTargetUniProbe; 
	struct DepthUBO
	{
		float u_Near;
		float u_Far;
		float u_Linear;
		float u_Reverse;
	}m_DepthUBO;*/

public:
	mars::Vec4 m_Colour;
	mars::Vec3 m_Position;
	mars::Vec3 m_Direction;

public:
	Light(void* device, LightType type, const mars::Vec3& position, const mars::Vec3& direction, const mars::Vec4& colour);
	~Light();

	void Specular(float shineFactor, float reflectivity);
	void Ambient(float ambientFactor);
	void Attenuation(float linear, float quadratic);
	void SpotCone(double theta);

	void Point();
	void Directional();
	void Spot();
	void Area();

	//void SetDepthParameters(float near, float far, bool linear = true, bool reverse = true);
	//void RenderDepthTexture(const std::deque<Object*>& renderQueue, int windowWidth, int windowHeight);
	//std::shared_ptr<OPENGL::Texture> GetDepthTexture();

	void UpdateColour();
	void UpdatePosition();
	void UpdateDirection();
	void UpdateDirection(double yaw, double pitch, double roll, bool invertYAxis);

	inline static void SetContext(miru::Ref<miru::crossplatform::Context> context)
	{
		GRAPHICS::UniformBuffer::SetContext(context);
	};
	std::shared_ptr<GRAPHICS::UniformBuffer> GetUBO0() { return m_UBO0; };
	std::shared_ptr<GRAPHICS::UniformBuffer> GetUBO1() { return m_UBO1; };

private:
	void InitialiseUBO();
	void SetAllToZero();
	//void SetDepthUBOToZero();
};
}
}