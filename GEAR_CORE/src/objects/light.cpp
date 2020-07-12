#include "light.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

int Light::s_NumOfLights = 0;

Light::Light(void* device, LightType type, const Vec3& position, const Vec3& direction, const Vec4& colour)
	:m_Device(device), m_Type(type), m_Position(position), m_Direction(direction), m_Colour(colour)
{
	InitialiseUB();
	s_NumOfLights++;
	if (s_NumOfLights < GEAR_MAX_LIGHTS + 1)
	{
		m_LightID = s_NumOfLights - 1;

		if (m_Type == LightType::GEAR_LIGHT_POINT)
			Point();
		else if (m_Type == LightType::GEAR_LIGHT_DIRECTIONAL)
			Directional();
		else if (m_Type == LightType::GEAR_LIGHT_SPOT)
			Spot();
		else if (m_Type == LightType::GEAR_LIGHT_AREA)
			Area();
	}
	else
	{
		GEAR_WARN(GEAR_ERROR_CODE::GEAR_OBJECTS | GEAR_ERROR_CODE::GEAR_INIT_FAILED, "ERROR: GEAR::OBJECTS::Light: Too many lights declared.");
	}
	UpdatePosition();
	UpdateColour();
	UpdateDirection();

	m_LightingUB.u_Diffuse = 1.0f;
	m_LightingUB.u_Specular = 1.0f;
	m_LightingUB.u_Ambient= 1.0f;
	m_LightingUB.u_Emit= 0.0f;
	m_UB1->SubmitData(&m_LightingUB.u_Diffuse, sizeof(LightingUB));
}

Light::~Light()
{
	s_NumOfLights--;
}

void Light::Specular(float shineFactor, float reflectivity)
{
	m_LightUB.m_ShineFactor = shineFactor;
	m_LightUB.m_Reflectivity = reflectivity;
	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}
void Light::Ambient(float ambientFactor)
{
	m_LightUB.m_AmbientFactor = ambientFactor;
	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}
void Light::Attenuation(float linear, float quadratic)
{
	m_LightUB.m_AttenuationConstant = 1.0f;
	m_LightUB.m_AttenuationLinear = linear;
	m_LightUB.m_AttenuationQuadratic = quadratic;
	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}
void Light::SpotCone(double theta)
{
	m_LightUB.m_CutOff = static_cast<float>(cos(theta));
	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}

void Light::Point()
{
	m_LightUB.m_Type = (float)LightType::GEAR_LIGHT_POINT;
	m_LightUB.m_Colour = m_Colour;
	m_LightUB.m_Position = Vec4(m_Position, 1.0f);
	/*m_DepthRenderTargetOmniProbe = std::make_unique<OmniProbe>(m_Position, m_DepthRenderSize);
	m_DepthRenderTargetUniProbe = nullptr;*/

	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}
void Light::Directional()
{
	m_LightUB.m_Type = (float)LightType::GEAR_LIGHT_DIRECTIONAL;
	m_LightUB.m_Colour = m_Colour;
	m_LightUB.m_Direction = Vec4(m_Direction, 0.0f);
	/*m_DepthRenderTargetOmniProbe = nullptr;
	m_DepthRenderTargetUniProbe = std::make_unique<UniProbe>(m_Position, m_Direction, m_DepthRenderSize, GEAR_CAMERA_ORTHOGRAPHIC);*/

	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}
void Light::Spot()
{
	m_LightUB.m_Type = (float)LightType::GEAR_LIGHT_SPOT;
	m_LightUB.m_Colour = m_Colour;
	m_LightUB.m_Position = Vec4(m_Position, 1.0f);
	m_LightUB.m_Direction = Vec4(m_Direction, 0.0f);
	/*m_DepthRenderTargetOmniProbe = nullptr;
	m_DepthRenderTargetUniProbe = std::make_unique<UniProbe>(m_Position, m_Direction, m_DepthRenderSize, GEAR_CAMERA_PERSPECTIVE);*/

	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}
void Light::Area()
{
	m_LightUB.m_Type = (float)LightType::GEAR_LIGHT_AREA;
	m_LightUB.m_Colour = m_Colour;

	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}

/*void Light::SetDepthParameters(float near, float far, bool linear, bool reverse)
{
	m_DepthUBO.u_Near = near;
	m_DepthUBO.u_Far = far;
	m_DepthUBO.u_Linear = static_cast<float>(linear);
	m_DepthUBO.u_Reverse = static_cast<float>(reverse);

	OPENGL::BufferManager::UpdateUBO(5, &m_DepthUBO, sizeof(DepthUBO), 0);
}
void Light::RenderDepthTexture(const std::deque<Object*>& renderQueue, int windowWidth, int windowHeight)
{
	if (m_Type == LightType::GEAR_LIGHT_POINT && m_DepthRenderTargetOmniProbe)
	{
		m_DepthRenderTargetOmniProbe->UpdatePosition(m_Position);
		m_DepthRenderTargetOmniProbe->Render(renderQueue, windowWidth, windowHeight, &m_DepthShader); return;
	}
	else if ((m_Type == LightType::GEAR_LIGHT_SPOT || m_Type == LightType::GEAR_LIGHT_DIRECTIONAL) && m_DepthRenderTargetUniProbe)
	{
		m_DepthRenderTargetUniProbe->UpdatePosition(m_Position);
		m_DepthRenderTargetUniProbe->Render(renderQueue, windowWidth, windowHeight, &m_DepthShader); return;
	}
	else
		return;
}
std::shared_ptr<OPENGL::Texture> Light::GetDepthTexture()
{
	if (m_Type == LightType::GEAR_LIGHT_POINT)
		return m_DepthRenderTargetOmniProbe->GetCubemap();
	else if (m_Type == LightType::GEAR_LIGHT_SPOT || m_Type == LightType::GEAR_LIGHT_DIRECTIONAL)
		return m_DepthRenderTargetUniProbe->GetTexture();
	else
		return nullptr;
}*/

void Light::UpdateColour()
{
	m_LightUB.m_Colour = m_Colour;
	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}
void Light::UpdatePosition()
{
	m_LightUB.m_Position = Vec4(m_Position, 1.0f);
	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}
void Light::UpdateDirection()
{
	m_LightUB.m_Direction = Vec4(m_Direction, 0.0f);
	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}
void Light::UpdateDirection(double yaw, double pitch, double roll, bool invertYAxis)
{
	double Pitch = pitch;
	if (invertYAxis == true)
	{
		Pitch = -pitch;
	}
	Vec3 direction;

	direction.x = static_cast<float>(cos(Pitch) * sin(yaw));
	direction.y = static_cast<float>(sin(Pitch));
	direction.z = static_cast<float>(cos(Pitch) * -cos(yaw));

	m_Direction = Vec3::Normalise(direction);

	m_LightUB.m_Direction = Vec4(m_Direction, 0.0f);
	m_UB0->SubmitData(&m_LightUB.m_Colour.r, sizeof(LightUB));
}

void Light::InitialiseUB()
{
	float zero0[sizeof(LightUB) * GEAR_MAX_LIGHTS] = { 0 };
	float zero1[sizeof(LightingUB)] = { 0 };
	
	UniformBuffer::CreateInfo ubCI;
	ubCI.device = m_Device;
	ubCI.data = zero0;
	ubCI.size = sizeof(LightUB)* GEAR_MAX_LIGHTS;
	m_UB0 = gear::CreateRef<UniformBuffer>(&ubCI);
	
	ubCI.data = zero1;
	ubCI.size = sizeof(LightingUB);
	m_UB1 = gear::CreateRef<UniformBuffer>(&ubCI);
}