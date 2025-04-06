#include "gear_core_common.h"
#include "Objects/Probe.h"
#include "Objects/Light.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

uint32_t Light::s_NumOfLights = 0;

typedef graphics::UniformBufferStructures::Lights LightUB;
Ref<graphics::Uniformbuffer<LightUB>> Light::s_UB = nullptr;

Light::Light(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseUB();

	if (s_NumOfLights < s_MaxLights)
	{
		m_LightID = s_NumOfLights;
		s_NumOfLights++;

		Update(Transform());
	}
	else
	{
		GEAR_WARN(ErrorCode::OBJECTS | ErrorCode::INIT_FAILED, "Too many lights declared.");
	}

	CreateProbe();
}

Light::~Light()
{
	s_UB->lights[m_LightID].type_valid_spotInner_spotOuter.y = 0.0f;
	s_NumOfLights--;
	
	if(!s_NumOfLights)
		s_UB = nullptr;
}

void Light::Update(const Transform& transform)
{
	if (CreateInfoHasChanged(&m_CI))
	{
		s_UB->lights[m_LightID].colour = m_CI.colour;
		s_UB->lights[m_LightID].type_valid_spotInner_spotOuter.x = static_cast<float>(m_CI.type);
		s_UB->lights[m_LightID].type_valid_spotInner_spotOuter.z = m_CI.spotInnerAngle;
		s_UB->lights[m_LightID].type_valid_spotInner_spotOuter.w = m_CI.spotOuterAngle;
		CreateProbe();
	}
	if (TransformHasChanged(transform))
	{
		s_UB->lights[m_LightID].position = float4(transform.translation, 1.0f);
		s_UB->lights[m_LightID].direction = transform.orientation.ToRotationMatrix4<float>() * float4(0, 0, 1, 0);
	}
	s_UB->lights[m_LightID].type_valid_spotInner_spotOuter.y = 1.0f;
	if (GetUpdateGPUFlag())
	{
		s_UB->SubmitData();
	}

	m_Probe->Update(transform);
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
	newHash ^= core::GetHash(CI.spotInnerAngle);
	newHash ^= core::GetHash(CI.spotOuterAngle);
	newHash ^= core::GetHash(CI.viewCamera);
	return CompareCreateInfoHash(newHash);
}

void Light::InitialiseUB()
{
	if (!s_UB)
	{
		LightUB zero0 = {};

		Uniformbuffer<LightUB>::CreateInfo ubCI;
		ubCI.debugName = "GEAR_CORE_Light_LightUBType: " + m_CI.debugName;
		ubCI.device = m_CI.device;
		ubCI.data = &zero0;
		s_UB = CreateRef<Uniformbuffer<LightUB>>(&ubCI);
	}
}

void Light::CreateProbe()
{
	Probe::CreateInfo probeCI;
	probeCI.debugName = "GEAR_CORE_Probe_Light " + std::to_string(m_LightID) + ": " + m_CI.debugName;
	probeCI.device = m_CI.device;
	probeCI.directionType = m_CI.type == Type::POINT ? Probe::DirectionType::OMNI : Probe::DirectionType::MONO;
	probeCI.captureType = Probe::CaptureType::SHADOW;
	probeCI.imageSize = 512;
	probeCI.projectionType = m_CI.type == Type::DIRECTIONAL ? Camera::ProjectionType::ORTHOGRAPHIC : Camera::ProjectionType::PERSPECTIVE;
	probeCI.perspectiveHorizonalFOV = m_CI.type == Type::POINT ? pi / 2.0 : m_CI.type == Type::SPOT ? m_CI.spotOuterAngle * 2.0 : 0.0;
	probeCI.zNear = 0.001f;
	probeCI.zFar = 3000.0f;
	probeCI.viewCamera = m_CI.viewCamera;
	if (m_CI.type == Type::DIRECTIONAL)
	{
		probeCI.shadowCascades = Probe::MaxShadowCascades;
		probeCI.shadowCascadeDistances[0] = 0.0f;
		probeCI.shadowCascadeDistances[1] = 0.0f;
		probeCI.shadowCascadeDistances[2] = 0.0f;
		probeCI.shadowCascadeDistances[3] = 0.0f;
	}
	else
	{
		probeCI.shadowCascades = 1;
		probeCI.shadowCascadeDistances[0] = 3000.0f;
		probeCI.shadowCascadeDistances[1] = 0.0f;
		probeCI.shadowCascadeDistances[2] = 0.0f;
		probeCI.shadowCascadeDistances[3] = 0.0f;
	}
	probeCI.shadowCascadeSplitLambda = 0.99f;
	probeCI.calculateShadowCascadeDistances = true;
	m_Probe = CreateRef<Probe>(&probeCI);
}
