#include "light.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

int Light::s_NumOfLights = 0;
bool Light::s_InitialiseUBO = false;

Light::Light(void* device, LightType type, const Vec3& position, const Vec3& direction, const Vec4& colour)
	:m_Device(device), m_Type(type), m_Position(position), m_Direction(direction), m_Colour(colour)
{
	InitialiseUBO();
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
		std::cout << "ERROR: GEAR::GRAPHICS::OBJECTS::Light: Too many lights declared!" << std::endl;
	}
	UpdatePosition();
	UpdateColour();
	UpdateDirection();

	m_LightingUBO.u_Diffuse = 1.0f;
	m_LightingUBO.u_Specular = 1.0f;
	m_LightingUBO.u_Ambient= 1.0f;
	m_LightingUBO.u_Emit= 0.0f;
	m_UBO1->SubmitData(&m_LightingUBO.u_Diffuse, sizeof(LightingUBO));
}

Light::~Light()
{
	SetAllToZero();
	s_NumOfLights--;
}

void Light::Specular(float shineFactor, float reflectivity)
{
	m_LightUBO.m_ShineFactor = shineFactor;
	m_LightUBO.m_Reflectivity = reflectivity;
	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}
void Light::Ambient(float ambientFactor)
{
	m_LightUBO.m_AmbientFactor = ambientFactor;
	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}
void Light::Attenuation(float linear, float quadratic)
{
	m_LightUBO.m_AttenuationConstant = 1.0f;
	m_LightUBO.m_AttenuationLinear = linear;
	m_LightUBO.m_AttenuationQuadratic = quadratic;
	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}
void Light::SpotCone(double theta)
{
	m_LightUBO.m_CutOff = static_cast<float>(cos(theta));
	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}

void Light::Point()
{
	m_LightUBO.m_Type = (float)LightType::GEAR_LIGHT_POINT;
	m_LightUBO.m_Colour = m_Colour;
	m_LightUBO.m_Position = Vec4(m_Position, 1.0f);
	/*m_DepthRenderTargetOmniProbe = std::make_unique<OmniProbe>(m_Position, m_DepthRenderSize);
	m_DepthRenderTargetUniProbe = nullptr;*/

	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}
void Light::Directional()
{
	m_LightUBO.m_Type = (float)LightType::GEAR_LIGHT_DIRECTIONAL;
	m_LightUBO.m_Colour = m_Colour;
	m_LightUBO.m_Direction = Vec4(m_Direction, 0.0f);
	/*m_DepthRenderTargetOmniProbe = nullptr;
	m_DepthRenderTargetUniProbe = std::make_unique<UniProbe>(m_Position, m_Direction, m_DepthRenderSize, GEAR_CAMERA_ORTHOGRAPHIC);*/

	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}
void Light::Spot()
{
	m_LightUBO.m_Type = (float)LightType::GEAR_LIGHT_SPOT;
	m_LightUBO.m_Colour = m_Colour;
	m_LightUBO.m_Position = Vec4(m_Position, 1.0f);
	m_LightUBO.m_Direction = Vec4(m_Direction, 0.0f);
	/*m_DepthRenderTargetOmniProbe = nullptr;
	m_DepthRenderTargetUniProbe = std::make_unique<UniProbe>(m_Position, m_Direction, m_DepthRenderSize, GEAR_CAMERA_PERSPECTIVE);*/

	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}
void Light::Area()
{
	m_LightUBO.m_Type = (float)LightType::GEAR_LIGHT_AREA;
	m_LightUBO.m_Colour = m_Colour;

	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
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
	m_LightUBO.m_Colour = m_Colour;
	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}
void Light::UpdatePosition()
{
	m_LightUBO.m_Position = Vec4(m_Position, 1.0f);
	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}
void Light::UpdateDirection()
{
	m_LightUBO.m_Direction = Vec4(m_Direction, 0.0f);
	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
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

	m_LightUBO.m_Direction = Vec4(m_Direction, 0.0f);
	m_UBO0->SubmitData(&m_LightUBO.m_Colour.r, sizeof(LightUBO));
}

void Light::InitialiseUBO()
{
	if (s_InitialiseUBO == false)
	{
		m_UBO0 = std::make_shared<UniformBuffer>(m_Device, sizeof(LightUBO) * GEAR_MAX_LIGHTS, 2);
		m_UBO1 = std::make_shared<UniformBuffer>(m_Device, sizeof(LightingUBO), 3);
		s_InitialiseUBO = true;
		{
			const float zero[sizeof(LightUBO) * GEAR_MAX_LIGHTS] = { 0 };
			m_UBO0->SubmitData(zero, sizeof(LightUBO) * GEAR_MAX_LIGHTS);
		}
		{
			const float zero[sizeof(LightingUBO)] = { 0 };
			m_UBO1->SubmitData(zero, sizeof(LightingUBO));
		}
	}
}
void Light::SetAllToZero()
{
	const float zero[sizeof(LightUBO)] = { 0 };
	m_UBO0->SubmitData(zero, sizeof(LightUBO));
}
