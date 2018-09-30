#include "camera.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Camera::Camera(int projType, const Shader& shader, const ARM::Vec3& position, const ARM::Vec3& forward, const ARM::Vec3 up)
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
}

void Camera::DefineView()
{
	//m_ViewMatrix = CameraMatrix(m_Position, m_Position + m_Forward, m_Up); TODO: Corect the lookAt matrix or Quaternions
	m_ViewMatrix =
		Mat4::Rotation(m_Pitch, m_xAxis) *
		Mat4::Rotation(m_Yaw, m_yAxis) *
		Mat4::Rotation(m_Roll, m_zAxis) *
		Mat4::Translation(Vec3(-m_Position.x, -m_Position.y, -m_Position.z));

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
void Camera::UpdateProjectionAndViewInOtherShader(const Shader& shader)
{
	shader.Enable();
	shader.SetUniformMatrix<4>("u_Proj", 1, GL_TRUE, m_ProjectionMatrix.a);
	shader.SetUniformMatrix<4>("u_View", 1, GL_TRUE, m_ViewMatrix.a);
	shader.Disable();
}

//private:
void Camera::CalculateRight()
{
	m_Right = Vec3::Normalise(Vec3::Cross(m_Forward, m_Up));
}
Mat4 Camera::CameraMatrix(const Vec3& camPos, const Vec3& camTarget, const Vec3& up)
{
	Vec3 m_Forward = Vec3::Normalise(camTarget - camPos);
	Vec3 m_Right = Vec3::Normalise(Vec3::Cross(m_Forward, up));
	Vec3 m_Up = Vec3::Normalise(Vec3::Cross(m_Forward, m_Right));
	
	Mat4 rotation(Vec4( m_Right.x,    m_Right.y,    m_Right.z,   0),
				  Vec4( m_Up.x,       m_Up.y,       m_Up.z,      0),
				  Vec4(-m_Forward.x, -m_Forward.y, -m_Forward.z, 0),
				  Vec4(0, 0, 0, 1));
	Mat4 translation = Mat4::Translation(Vec3(-camPos.x, -camPos.y, -camPos.z));
	Mat4 output = rotation * translation;
	return output;
}