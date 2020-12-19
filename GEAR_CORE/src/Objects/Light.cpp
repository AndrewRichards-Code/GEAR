#include "gear_core_common.h"
#include "Light.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

int Light::s_NumOfLights = 0;

typedef graphics::UniformBufferStructures::Lights LightUB;
 gear::Ref<graphics::Uniformbuffer<LightUB>> Light::s_UB = nullptr;

Light::Light(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseUB();
	if (s_NumOfLights < GEAR_MAX_LIGHTS)
	{
		m_LightID = s_NumOfLights;
		s_NumOfLights++;

		Update();
	}
	else
	{
		GEAR_LOG(core::Log::Level::WARN, core::Log::ErrorCode::OBJECTS | core::Log::ErrorCode::INIT_FAILED, "Too many lights declared.");
	}
}

Light::~Light()
{
	s_UB->lights[m_LightID].valid = Vec4(0, 0, 0, 0);
	s_NumOfLights--;
}

void Light::Update()
{
	s_UB->lights[m_LightID].colour = m_CI.colour;
	s_UB->lights[m_LightID].position = Vec4(m_CI.transform.translation, 1.0f);
	s_UB->lights[m_LightID].direction = m_CI.transform.orientation.ToMat4() * Vec4(0, 0, -1, 0);
	s_UB->lights[m_LightID].valid = Vec4(1, 1, 1, 1);
	s_UB->SubmitData();
}

void Light::InitialiseUB()
{
	if (!s_UB)
	{
		float zero0[sizeof(LightUB)] = { 0 };

		Uniformbuffer<LightUB>::CreateInfo ubCI;
		ubCI.debugName = "GEAR_CORE_Light_LightUBType: " + m_CI.debugName;
		ubCI.device = m_CI.device;
		ubCI.data = zero0;
		s_UB = gear::CreateRef<Uniformbuffer<LightUB>>(&ubCI);
	}
}