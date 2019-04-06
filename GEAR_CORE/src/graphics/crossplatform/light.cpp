#include "light.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace CROSSPLATFORM;
using namespace ARM;

int Light::s_NumOfLights = 0;
bool Light::s_InitialiseUBO = false;

Light::Light(LightType type, const Vec3& position, const Vec3& direction, const Vec4& colour, OPENGL::Shader& shader)
	:m_Type(type), m_Position(position), m_Direction(direction), m_Colour(colour), m_Shader(shader)
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
		std::cout << "ERROR: GEAR::GRAPHICS::CROSSPLATFORM::Light: Too many lights declared!" << std::endl;
	}
	//m_LightCam.DefineProjection(-10, 10, -10, 10, -10, 20);
	UpdatePosition();
	UpdateColour();
	UpdateDirection();
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
	OPENGL::BufferManager::UpdateUBO(2, &m_LightUBO.m_ShineFactor, 2 * sizeof(float), offsetof(LightUBO, m_ShineFactor) + m_LightID * sizeof(LightUBO));
}

void Light::Ambient(float ambientFactor)
{
	m_LightUBO.m_AmbientFactor = ambientFactor;
	OPENGL::BufferManager::UpdateUBO(2, &m_LightUBO.m_AmbientFactor, sizeof(float), offsetof(LightUBO, m_AmbientFactor) + m_LightID * sizeof(LightUBO));
}

void Light::Attenuation(float linear, float quadratic)
{
	m_LightUBO.m_AttenuationConstant = 1.0f;
	m_LightUBO.m_AttenuationLinear = linear;
	m_LightUBO.m_AttenuationQuadratic = quadratic;
	OPENGL::BufferManager::UpdateUBO(2, &m_LightUBO.m_AttenuationConstant, 3 * sizeof(float), offsetof(LightUBO, m_AttenuationConstant) + m_LightID * sizeof(LightUBO));
}

void Light::SpotCone(double theta)
{
	m_LightUBO.m_CutOff = static_cast<float>(cos(theta));
	OPENGL::BufferManager::UpdateUBO(2, &m_LightUBO.m_CutOff, sizeof(float), offsetof(LightUBO, m_CutOff) + m_LightID * sizeof(LightUBO));
}

void Light::Point()
{
	m_LightUBO.m_Type = (float)LightType::GEAR_LIGHT_POINT;
	m_LightUBO.m_Colour = m_Colour;
	m_LightUBO.m_Position = Vec4(m_Position, 1.0f);

	OPENGL::BufferManager::UpdateUBO(2, &m_LightUBO.m_Type, sizeof(float), offsetof(LightUBO, m_Type) + m_LightID * sizeof(LightUBO));
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Colour, sizeof(Vec4), offsetof(LightUBO, m_Colour) + m_LightID * sizeof(LightUBO));
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Position, sizeof(Vec4), offsetof(LightUBO, m_Position) + m_LightID * sizeof(LightUBO));
}
void Light::Directional()
{
	m_LightUBO.m_Type = (float)LightType::GEAR_LIGHT_DIRECTIONAL;
	m_LightUBO.m_Colour = m_Colour;
	m_LightUBO.m_Direction = Vec4(m_Direction, 0.0f);

	OPENGL::BufferManager::UpdateUBO(2, &m_LightUBO.m_Type, sizeof(float), offsetof(LightUBO, m_Type) + m_LightID * sizeof(LightUBO));
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Colour, sizeof(Vec4), offsetof(LightUBO, m_Colour) + m_LightID * sizeof(LightUBO));
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Direction, sizeof(Vec4), offsetof(LightUBO, m_Direction) + m_LightID * sizeof(LightUBO));
}
void Light::Spot()
{
	m_LightUBO.m_Type = (float)LightType::GEAR_LIGHT_SPOT;
	m_LightUBO.m_Colour = m_Colour;
	m_LightUBO.m_Position = Vec4(m_Position, 1.0f);
	m_LightUBO.m_Direction = Vec4(m_Direction, 0.0f);

	OPENGL::BufferManager::UpdateUBO(2, &m_LightUBO.m_Type, sizeof(float), offsetof(LightUBO, m_Type) + m_LightID * sizeof(LightUBO));
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Colour, sizeof(Vec4), offsetof(LightUBO, m_Colour) + m_LightID * sizeof(LightUBO));
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Position, sizeof(Vec4), offsetof(LightUBO, m_Position) + m_LightID * sizeof(LightUBO));
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Direction, sizeof(Vec4), offsetof(LightUBO, m_Direction) + m_LightID * sizeof(LightUBO));
}
void Light::Area()
{
	m_LightUBO.m_Type = (float)LightType::GEAR_LIGHT_AREA;
	m_LightUBO.m_Colour = m_Colour;

	OPENGL::BufferManager::UpdateUBO(2, &m_LightUBO.m_Type, sizeof(float), offsetof(LightUBO, m_Type) + m_LightID * sizeof(LightUBO));
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Colour, sizeof(Vec4), offsetof(LightUBO, m_Colour) + m_LightID * sizeof(LightUBO));
}

void Light::CalculateLightMVP()
{
	/*m_LightCam.UpdateCameraPosition();
	m_LightCam.CalcuateLookAround(0, 0, 0, true);
	m_LightCam.DefineView();*/
}

void Light::UpdateColour()
{
	m_LightUBO.m_Colour = m_Colour;
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Colour, sizeof(Vec4), offsetof(LightUBO, m_Colour) + m_LightID * sizeof(LightUBO));
}

void Light::UpdatePosition()
{
	m_LightUBO.m_Position = Vec4(m_Position, 1.0f);
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Position, sizeof(Vec4), offsetof(LightUBO, m_Position) + m_LightID * sizeof(LightUBO));
}

void Light::UpdateDirection()
{
	m_LightUBO.m_Direction = Vec4(m_Direction, 0.0f);
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Direction, sizeof(Vec4), offsetof(LightUBO, m_Direction) + m_LightID * sizeof(LightUBO));
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
	OPENGL::BufferManager::UpdateUBO(2, (const float*)&m_LightUBO.m_Direction, sizeof(Vec4), offsetof(LightUBO, m_Direction) + m_LightID * sizeof(LightUBO));
}

void Light::InitialiseUBO()
{
	if (s_InitialiseUBO == false)
	{
		OPENGL::BufferManager::AddUBO(sizeof(LightUBO) * GEAR_MAX_LIGHTS, 2);
		s_InitialiseUBO = true;

		const float zero[sizeof(LightUBO) * GEAR_MAX_LIGHTS] = { 0 };
		OPENGL::BufferManager::UpdateUBO(2, zero, sizeof(LightUBO) * GEAR_MAX_LIGHTS, 0);
	}
}

void Light::SetAllToZero()
{
	const float zero[sizeof(LightUBO)] = { 0 };
	OPENGL::BufferManager::UpdateUBO(2, zero, sizeof(LightUBO), m_LightID * sizeof(LightUBO));
}
