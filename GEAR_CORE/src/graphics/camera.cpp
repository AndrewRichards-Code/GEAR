#include "camera.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Camera::Camera(int projType, Shader& shader, const ARM::Vec3& position, const ARM::Vec3& forward, const ARM::Vec3 up)
	:m_ProjectionType(projType), m_Shader(shader), m_Position(position), m_Forward(forward), m_Up(up)
{
	m_Forward.Normalise();
	m_Up.Normalise();
	CalculateRight();

	UpdateCameraPosition();
	DefineView();
}

Camera::~Camera()
{
}

//private:
void Camera::CalculateRight()
{
	m_Right = Vec3::Normalise(Vec3::Cross(m_Forward, m_Up));
}
void Camera::Move(const ARM::Vec3 & direction, float value)
{
	m_Position = m_Position + (direction * value);
}
void Camera::RotateX(double pitch)
{
	Vec3 horizonalAxis = Vec3::Normalise(Vec3::Cross(m_Forward, m_yAxis));
	m_Forward.RotQuat(pitch, horizonalAxis);
	m_Up = Vec3::Normalise(Vec3::Cross(horizonalAxis, m_Forward));

}
void Camera::RotateY(double yaw)
{
	Vec3 horizonalAxis = Vec3::Normalise(Vec3::Cross(m_Forward, m_yAxis));
	m_Forward.RotQuat(yaw, m_yAxis);
	m_Up = Vec3::Normalise(Vec3::Cross(m_Forward, horizonalAxis));

}

Mat4 Camera::CameraMatrix(const Vec3& camPos, const Vec3& camTarget, const Vec3& up)
{
	Vec3 m_Forward = Vec3::Normalise(camTarget - camPos);
	Vec3 m_Right = Vec3::Normalise(Vec3::Cross(m_Forward, up));
	Vec3 m_Up = Vec3::Normalise(Vec3::Cross(m_Forward, m_Right));
	
	Mat4 output(Vec4( m_Right.x,    m_Right.y,    m_Right.z,   -Vec3::Dot(m_Right, camPos)),
				Vec4( m_Up.x,       m_Up.y,       m_Up.z,      -Vec3::Dot(m_Up, camPos)),
				Vec4(-m_Forward.x, -m_Forward.y, -m_Forward.z,  Vec3::Dot(m_Forward, camPos)),
				Vec4(0,             0,            0,            1));
	return output;
}

//public:
void Camera::UpdateCameraPosition()
{
	m_Shader.Enable();
	m_Shader.SetUniform<float>("u_CameraPosition", { m_Position.x,  m_Position.y,  m_Position.z });
	m_Shader.Disable();
}
void Camera::CalcuateLookAround(double yaw, double pitch, double roll, bool invertYAxis)
{
	m_Yaw = yaw; 
	if (invertYAxis == true)
	{
		m_Pitch = -pitch;
	}
	m_Pitch = pitch; 
	m_Roll = roll;
	Vec3 direction;

	direction.x = static_cast<float>(cos(m_Pitch) * sin(m_Yaw));
	direction.y = static_cast<float>(sin(m_Pitch));
	direction.z = static_cast<float>(cos(m_Pitch) * -cos(m_Yaw));

	m_Forward = Vec3::Normalise(direction);
	Vec3 m_Right = Vec3::Normalise(Vec3::Cross(m_Forward, m_yAxis));
	Vec3 m_Up = Vec3::Normalise(Vec3::Cross(m_Forward, m_Right));

	//RotateX(m_Pitch);
	//RotateY(m_Yaw);
}

void Camera::DefineView()
{
	m_ViewMatrix = CameraMatrix(m_Position, m_Position + m_Forward, m_Up);
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
void Camera::DefineProjection(double fov, float aspectRatio, float zNear, float zFar)
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