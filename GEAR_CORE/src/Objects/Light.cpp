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
		
		m_UB->at(m_LightID).colour = m_CI.colour;
		m_UB->at(m_LightID).position = Vec4(m_CI.position, 1.0f);
		m_UB->at(m_LightID).direction = Vec4(m_CI.direction, 0.0f);
		m_UB->SubmitData();
	}
	else
	{
		GEAR_WARN(GEAR_ERROR_CODE::GEAR_OBJECTS | GEAR_ERROR_CODE::GEAR_INIT_FAILED, "ERROR: GEAR::OBJECTS::Light: Too many lights declared.");
	}
}

Light::~Light()
{
	s_NumOfLights--;
}

void gear::objects::Light::Update()
{
	m_UB->at(m_LightID).colour = m_CI.colour;
	m_UB->at(m_LightID).position = Vec4(m_CI.position, 1.0f);
	m_UB->at(m_LightID).direction = Vec4(m_CI.direction, 0.0f);
	m_UB->SubmitData();
}

void Light::InitialiseUB()
{
	float zero0[sizeof(LightUBType)] = { 0 };
	
	Uniformbuffer<LightUBType>::CreateInfo ubCI;
	ubCI.debugName = m_DebugName.c_str();
	ubCI.device = m_CI.device;
	ubCI.data = zero0;
	m_UB = gear::CreateRef<Uniformbuffer<LightUBType>>(&ubCI);
}