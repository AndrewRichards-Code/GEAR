#include "light.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace CROSSPLATFORM;
using namespace ARM;

int Light::s_NumOfLights = 0;

Light::Light(int type, const ARM::Vec3& position, ARM::Vec3& direction, const ARM::Vec4& colour, const Shader& shader)
	:m_Type(type), m_Position(position), m_Direction(direction), m_Colour(colour), m_Shader(shader)
{
	s_NumOfLights++;
	if (s_NumOfLights < GEAR_MAX_LIGHTS + 1)
	{
		m_LightID = s_NumOfLights - 1;

		if (m_Type == GEAR_LIGHT_POINT)
			Point();
		else if (m_Type == GEAR_LIGHT_DIRECTIONAL)
			Directional();
		else if (m_Type == GEAR_LIGHT_SPOT)
			Spot();
		else if (m_Type == GEAR_LIGHT_AREA)
			Area();
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::CROSSPLATFORM::Light: Too many lights declared!" << std::endl;
	}
	//m_LightCam.DefineProjection(-10, 10, -10, 10, -10, 20);
}

Light::~Light()
{
	s_NumOfLights--;
}

void Light::Specular(float shineFactor, float reflectivity) const
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].ShineFactor", { shineFactor });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Reflectivity", { reflectivity });
	m_Shader.Disable();
}

void Light::Ambient(float ambientFactor) const
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].AmbientFactor", { ambientFactor });
	m_Shader.Disable();
}

void Light::Attenuation(float linear, float quadratic) const
{
	float constant = 1.0f;
	
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].AttenuationConstant", { constant });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].AttenuationLinear", { linear });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].AttenuationQuadratic",  { quadratic });
	m_Shader.Disable();
}

void Light::SpotCone(double theta) const
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].CutOff", { static_cast<float>(cos(theta)) });
	m_Shader.Disable();
}

void Light::Point() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<int>(  "u_Lights[" + std::to_string(m_LightID) + "].Type", { GEAR_LIGHT_POINT });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Colour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Position", { m_Position.x, m_Position.y, m_Position.z });
	m_Shader.Disable();
}
void Light::Directional() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<int>(  "u_Lights[" + std::to_string(m_LightID) + "].Type", { GEAR_LIGHT_DIRECTIONAL });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Colour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Direction", { m_Direction.x, m_Direction.y, m_Direction.z });
	m_Shader.Disable();
}
void Light::Spot() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<int>(  "u_Lights[" + std::to_string(m_LightID) + "].Type", { GEAR_LIGHT_SPOT });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Colour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Position", { m_Position.x, m_Position.y, m_Position.z });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Direction", { m_Direction.x, m_Direction.y, m_Direction.z });
	m_Shader.Disable();
}
void Light::Area() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<int>(  "u_Lights[" + std::to_string(m_LightID) + "].Type", { GEAR_LIGHT_AREA });
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Colour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.Disable();
}

void Light::CalculateLightMVP()
{
	/*m_LightCam.UpdateCameraPosition();
	m_LightCam.CalcuateLookAround(0, 0, 0, true);
	m_LightCam.DefineView();*/
}

void Light::UpdateColour()
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Colour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.Disable();
}

void Light::UpdatePosition()
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Position", { m_Position.x, m_Position.y, m_Position.z });
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
	m_Shader.SetUniform<float>("u_Lights[" + std::to_string(m_LightID) + "].Direction", { m_Direction.x, m_Direction.y, m_Direction.z });
	m_Shader.Disable();
}