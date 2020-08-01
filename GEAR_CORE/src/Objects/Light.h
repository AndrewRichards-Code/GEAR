#pragma once

#include "Camera.h"

#define GEAR_MAX_LIGHTS 8

namespace gear {
namespace objects {

//TODO: Add Shadow mapping.
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
	
	struct CreateInfo
	{
		const char* debugName;
		void*		device;
		LightType	type;
		mars::Vec4	colour;
		mars::Vec3	position;
		mars::Vec3	direction;
	};

private:
	struct LightUB
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
	};										//80 Total Size
	typedef std::array<LightUB, GEAR_MAX_LIGHTS> LightUBType;
	gear::Ref<graphics::Uniformbuffer<LightUBType>> m_UB0;
	
	struct LightingUB
	{
		float u_Diffuse;
		float u_Specular;
		float u_Ambient;
		float u_Emit;
	};
	gear::Ref<graphics::Uniformbuffer<LightingUB>> m_UB1;
	
	static int s_NumOfLights;
	size_t m_LightID;
	LightType m_Type;

	std::string m_DebugName;

public:
	CreateInfo m_CI;

public:
	Light(CreateInfo* pCreateInfo);
	~Light();

	void Specular(float shineFactor, float reflectivity);
	void Ambient(float ambientFactor);
	void Attenuation(float linear, float quadratic);
	void SpotCone(double theta);

	//Update the camera the current static of Camera::CreateInfo m_CI.
	void Update();
	gear::Ref<graphics::Uniformbuffer<LightUBType>> GetUB0() { return m_UB0; };
	gear::Ref<graphics::Uniformbuffer<LightingUB>> GetUB1() { return m_UB1; };

private:
	void InitialiseUB();
};
}
}