#include "light.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Light::Light(int type, const ARM::Vec3& position, ARM::Vec3& direction, const ARM::Vec4& colour, const Shader& shader)
	:m_Type(type), m_Position(position), m_Direction(direction), m_Colour(colour), m_Shader(shader)
{
	if (m_Type == GEAR_LIGHT_POINT)
		Point();
	else if (m_Type == GEAR_LIGHT_DIRECTIONAL)
		Directional();
	else if (m_Type == GEAR_LIGHT_SPOT)
		Spot();
	else if (m_Type == GEAR_LIGHT_AREA)
		Area();
	//m_LightCam.DefineProjection(-10, 10, -10, 10, -10, 20);
}

Light::~Light(){}

void Light::Specular(float shineFactor, float reflectivity) const
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_ShineFactor", { shineFactor });
	m_Shader.SetUniform<float>("u_Reflectivity", { reflectivity });
	m_Shader.Disable();
}

void Light::Ambient(float ambientFactor) const
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_AmbientFactor", { ambientFactor });
	m_Shader.Disable();
}

void Light::Attenuation(float linear, float quadratic) const
{
	float constant = 1.0f;
	
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_AttenuationConstant", { constant });
	m_Shader.SetUniform<float>("u_AttenuationLinear", { linear });
	m_Shader.SetUniform<float>("u_AttenuationQuadratic",  { quadratic });
	m_Shader.Disable();
}

void Light::SpotCone(double theta) const
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_CutOff", { static_cast<float>(cos(theta)) });
	m_Shader.Disable();
}

void Light::Point() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<int>("u_LightType", { GEAR_LIGHT_POINT });
	m_Shader.SetUniform<float>("u_LightColour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.SetUniform<float>("u_LightPosition", { m_Position.x, m_Position.y, m_Position.z });
	m_Shader.Disable();
}
void Light::Directional() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<int>("u_LightType", { GEAR_LIGHT_DIRECTIONAL });
	m_Shader.SetUniform<float>("u_LightColour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.SetUniform<float>("u_LightDirection", { m_Direction.x, m_Direction.y, m_Direction.z });
	m_Shader.Disable();
}
void Light::Spot() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<int>("u_LightType", { GEAR_LIGHT_SPOT });
	m_Shader.SetUniform<float>("u_LightColour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.SetUniform<float>("u_LightPosition", { m_Position.x, m_Position.y, m_Position.z });
	m_Shader.SetUniform<float>("u_LightDirection", { m_Direction.x, m_Direction.y, m_Direction.z });
	m_Shader.Disable();
}
void Light::Area() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<int>("u_LightType", { GEAR_LIGHT_AREA });
	m_Shader.SetUniform<float>("u_LightColour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.Disable();
}

void Light::CalculateLightMVP()
{
	/*m_LightCam.UpdateCameraPosition();
	m_LightCam.CalcuateLookAround(0, 0, 0, true);
	m_LightCam.DefineView();*/
}

void Light::UpdatePosition()
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_LightPosition", { m_Position.x, m_Position.y, m_Position.z });
	m_Shader.Disable();
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

	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_LightDirection", { m_Direction.x, m_Direction.y, m_Direction.z });
	m_Shader.Disable();
}
