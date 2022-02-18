#include "gear_core_common.h"
#include "Light.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

int Light::s_NumOfLights = 0;

typedef graphics::UniformBufferStructures::Lights LightUB;
 Ref<graphics::Uniformbuffer<LightUB>> Light::s_UB = nullptr;

Light::Light(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseUB();
	if (s_NumOfLights < GEAR_MAX_LIGHTS)
	{
		m_LightID = s_NumOfLights;
		s_NumOfLights++;

		Update(Transform());
	}
	else
	{
		GEAR_WARN(ErrorCode::OBJECTS | ErrorCode::INIT_FAILED, "Too many lights declared.");
	}
}

Light::~Light()
{
	s_UB->lights[m_LightID].valid = float4(0, 0, 0, 0);
	s_NumOfLights--;
	
	if(!s_NumOfLights)
		s_UB = nullptr;
}

void Light::Update(const Transform& transform)
{
	if (CreateInfoHasChanged(&m_CI))
	{
		s_UB->lights[m_LightID].colour = m_CI.colour;
	}
	if (TransformHasChanged(transform))
	{
		s_UB->lights[m_LightID].position = float4(transform.translation, 1.0f);
		s_UB->lights[m_LightID].direction = transform.orientation.ToRotationMatrix4<float>() * float4(0, 0, -1, 0);
	}
	s_UB->lights[m_LightID].valid = float4(1, 1, 1, 1);
	if (m_UpdateGPU)
	{
		s_UB->SubmitData();
	}
}

bool Light::CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo)
{
	const CreateInfo& CI = *reinterpret_cast<const CreateInfo*>(pCreateInfo);
	uint64_t newHash = 0;
	newHash ^= core::GetHash(CI.type);
	newHash ^= core::GetHash(CI.colour.r);
	newHash ^= core::GetHash(CI.colour.g);
	newHash ^= core::GetHash(CI.colour.b);
	newHash ^= core::GetHash(CI.colour.a);
	return CompareCreateInfoHash(newHash);
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
		s_UB = CreateRef<Uniformbuffer<LightUB>>(&ubCI);
	}
}