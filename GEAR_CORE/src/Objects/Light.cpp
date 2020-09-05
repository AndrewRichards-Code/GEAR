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

	InitialiseUB();
	s_NumOfLights++;
	if (s_NumOfLights < GEAR_MAX_LIGHTS + 1)
	{
		m_LightID = s_NumOfLights - 1;
		
		m_UB->at(m_LightID).colour = m_CI.colour;
		m_UB->at(m_LightID).position = Vec4(m_CI.transform.translation, 1.0f);
		m_UB->at(m_LightID).direction = m_CI.transform.orientation.ToMat4() * Vec4(0, 0, -1, 0);
		m_UB->SubmitData();
	}
	else
	{
		GEAR_LOG(core::Log::Level::WARN, core::Log::ErrorCode::OBJECTS | core::Log::ErrorCode::INIT_FAILED, "Too many lights declared.");
	}
}

Light::~Light()
{
	s_NumOfLights--;
}

void gear::objects::Light::Update()
{
	m_UB->at(m_LightID).colour = m_CI.colour;
	m_UB->at(m_LightID).position = Vec4(m_CI.transform.translation, 1.0f);
	m_UB->at(m_LightID).direction = Vec4(m_CI.transform.orientation.ToVec3(), 0.0f);
	m_UB->SubmitData();
}

void Light::InitialiseUB()
{
	float zero0[sizeof(LightUBType)] = { 0 };
	
	Uniformbuffer<LightUBType>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Light: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero0;
	m_UB = gear::CreateRef<Uniformbuffer<LightUBType>>(&ubCI);
}