#include "light.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Light::Light(int type, ARM::Vec3 pos_dir, ARM::Vec4 colour, Shader& shader)
	:m_Type(type), m_PosDir(pos_dir), m_Colour(colour), m_Shader(shader)
{
	if (m_Type == GEAR_LIGHT_POINT)
		Point();
	if (m_Type == GEAR_LIGHT_DIRECTIONAL)
		Directional();
	if (m_Type == GEAR_LIGHT_SPOT)
		Spot();
	if (m_Type == GEAR_LIGHT_AREA)
		Area();
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

void Light::Point() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_LightPosition", { m_PosDir.x, m_PosDir.y, m_PosDir.z });
	m_Shader.SetUniform<float>("u_LightColour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.Disable();
}
void Light::Directional() const
{
}
void Light::Spot() const
{
}
void Light::Area() const
{
}
