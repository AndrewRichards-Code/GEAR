#include "gear_core_common.h"
#include "Light.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

int Light::s_NumOfLights = 0;

Light::Light(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	m_DebugName = std::string("GEAR_CORE_Light: ") + m_CI.debugName;

	InitialiseUB();
	s_NumOfLights++;
	if (s_NumOfLights < GEAR_MAX_LIGHTS + 1)
	{
		m_LightID = s_NumOfLights - 1;

		m_UB0->at(m_LightID).m_Type = (float)m_CI.type;
		m_UB0->at(m_LightID).m_Colour = m_CI.colour;
		m_UB0->at(m_LightID).m_Position = Vec4(m_CI.position, 1.0f);
		m_UB0->at(m_LightID).m_Direction = Vec4(m_CI.direction, 0.0f);
		m_UB0->SubmitData();
	}
	else
	{
		GEAR_WARN(GEAR_ERROR_CODE::GEAR_OBJECTS | GEAR_ERROR_CODE::GEAR_INIT_FAILED, "ERROR: GEAR::OBJECTS::Light: Too many lights declared.");
	}

	m_UB1->u_Diffuse = 1.0f;
	m_UB1->u_Specular = 1.0f;
	m_UB1->u_Ambient= 1.0f;
	m_UB1->u_Emit= 0.0f;
	m_UB1->SubmitData();
}

Light::~Light()
{
	s_NumOfLights--;
}

void Light::Specular(float shineFactor, float reflectivity)
{
	m_UB0->at(m_LightID).m_ShineFactor = shineFactor;
	m_UB0->at(m_LightID).m_Reflectivity = reflectivity;
	m_UB0->SubmitData();
}
void Light::Ambient(float ambientFactor)
{
	m_UB0->at(m_LightID).m_AmbientFactor = ambientFactor;
	m_UB0->SubmitData();
}
void Light::Attenuation(float linear, float quadratic)
{
	m_UB0->at(m_LightID).m_AttenuationConstant = 1.0f;
	m_UB0->at(m_LightID).m_AttenuationLinear = linear;
	m_UB0->at(m_LightID).m_AttenuationQuadratic = quadratic;
	m_UB0->SubmitData();
}
void Light::SpotCone(double theta)
{
	m_UB0->at(m_LightID).m_CutOff = static_cast<float>(cos(theta));
	m_UB0->SubmitData();
}

void gear::objects::Light::Update()
{
	m_UB0->at(m_LightID).m_Colour = m_CI.colour;
	m_UB0->at(m_LightID).m_Position = Vec4(m_CI.position, 1.0f);
	m_UB0->at(m_LightID).m_Direction = Vec4(m_CI.direction, 0.0f);
	m_UB0->SubmitData();
}

void Light::InitialiseUB()
{
	float zero0[sizeof(LightUBType)] = { 0 };
	float zero1[sizeof(LightingUB)] = { 0 };
	
	Uniformbuffer<LightUBType>::CreateInfo ubCI0;
	ubCI0.debugName = m_DebugName.c_str();
	ubCI0.device = m_CI.device;
	ubCI0.data = zero0;
	m_UB0 = gear::CreateRef<Uniformbuffer<LightUBType>>(&ubCI0);
	
	Uniformbuffer<LightingUB>::CreateInfo ubCI1;
	ubCI1.debugName = m_DebugName.c_str();
	ubCI1.device = m_CI.device;
	ubCI1.data = zero1;
	m_UB1 = gear::CreateRef<Uniformbuffer<LightingUB>>(&ubCI1);
}