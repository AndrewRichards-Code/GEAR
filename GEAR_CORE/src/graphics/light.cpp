#include "light.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Light::Light(int type, const ARM::Vec3& pos_dir, const ARM::Vec4& colour, const Shader& shader)
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
	//m_LightCam.DefineProjection(-10, 10, -10, 10, -10, 20);
}

Light::~Light(){}

void Light::Specular(float shineFactor, float reflectivity) const
{
	m_LightParamsUBO.SubDataBind(&shineFactor, sizeof(float), offsetof(LightParams, m_ShineFactor));
	m_LightParamsUBO.SubDataBind(&reflectivity, sizeof(float), offsetof(LightParams, m_Reflectivity));
}

void Light::Ambient(float ambientFactor) const
{
	m_LightParamsUBO.SubDataBind(&ambientFactor, sizeof(float), offsetof(LightParams, m_AmbientFactor));
}

void Light::Point() const
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_LightPosition", { m_PosDir.x, m_PosDir.y, m_PosDir.z });
	m_Shader.Disable();
	m_LightParamsUBO.SubDataBind(&m_Colour.r, sizeof(Vec4), offsetof(LightParams, m_LightColour));
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

void Light::CalculateLightMVP()
{
	/*m_LightCam.UpdateCameraPosition();
	m_LightCam.CalcuateLookAround(0, 0, 0, true);
	m_LightCam.DefineView();*/
}
