#include "camera.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Camera::Camera(int projType, Shader& shader, Vec3 position, Vec3 forward, float yaw, float pitch, float roll)
	:m_ProjectionType(projType), m_Shader(shader), m_Position(position), m_Forward(forward), m_Yaw(yaw), m_Pitch(pitch), m_Roll(roll)
{
	UpdateCameraPosition();
	DefineView();
}

Camera::~Camera()
{
}

void Camera::UpdateCameraPosition()
{
	
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_CameraPosition", { m_Position.x,  m_Position.y,  m_Position.z });
	m_Shader.Disable();
}


void Camera::CalcuateLookAround(float yaw, float pitch, float roll)
{
	m_Yaw = yaw; 
	m_Pitch = pitch; 
	m_Roll = roll;
	Vec3 direction;

	direction.x = cos(m_Pitch) * sin(m_Yaw);
	direction.y = sin(m_Pitch);
	direction.z = cos(m_Pitch) * -cos(m_Yaw);

	m_Forward = Vec3::Normalise(direction);
}

void Camera::DefineView()
{
	m_ViewMatrix = LookAt(m_Position, m_Position + m_Forward, m_Up);
	m_Shader.Enable();
	m_Shader.SetUniformMatrix<4>("u_View", 1, GL_TRUE, m_ViewMatrix.a);
	m_Shader.Disable();
}

void Camera::DefineProjection(float left, float right, float bottom, float top, float near, float far)
{
	if (m_ProjectionType == GEAR_CAMERA_ORTHOGRAPHIC)
	{
		m_ProjectionMatrix = Mat4::Orthographic(left, right, bottom, top, near, far);
		m_Shader.Enable();
		m_Shader.SetUniformMatrix<4>("u_Proj", 1, GL_TRUE, m_ProjectionMatrix.a);
		m_Shader.Disable();
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::Camera: The parameters for DefinieProjection() don't match the projection type." << std::endl;
		throw;
	}
}
void Camera::DefineProjection(float fov, float aspectRatio, float zNear, float zFar)
{
	if (m_ProjectionType == GEAR_CAMERA_PERSPECTIVE)
	{
		m_ProjectionMatrix = Mat4::Perspective(fov, aspectRatio, zNear, zFar);
		m_Shader.Enable();
		m_Shader.SetUniformMatrix<4>("u_Proj", 1, GL_TRUE, m_ProjectionMatrix.a);
		m_Shader.Disable();
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::Camera: The parameters for DefinieProjection() don't match the projection type." << std::endl;
		throw;
	}
}

Mat4 Camera::LookAt(const Vec3& camPos, const Vec3& camTarget, const Vec3& up)
{
	Vec3 m_Forward = Vec3::Normalise(camTarget - camPos);
	Vec3 m_Right = Vec3::Normalise(Vec3::Cross(m_Forward, up));
	Vec3 m_Up = Vec3::Normalise(Vec3::Cross(m_Forward, m_Right));

	Mat4 output(Vec4(m_Right.x, m_Right.y, m_Right.z, -Vec3::Dot(m_Right, camPos)),
		Vec4(m_Up.x, m_Up.y, m_Up.z, -Vec3::Dot(m_Up, camPos)),
		Vec4(-m_Forward.x, -m_Forward.y, -m_Forward.z, -Vec3::Dot(m_Forward, camPos)),
		Vec4(0, 0, 0, 1));
	
	return output;
}
