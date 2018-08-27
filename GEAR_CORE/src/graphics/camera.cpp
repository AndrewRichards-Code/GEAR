#include "camera.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Camera::Camera(int projType, Shader& shader, Vec3 position, Vec3 front, float yaw, float pitch, float roll)
	:m_ProjectionType(projType), m_Shader(shader), m_Position(position), m_Front(front), m_Yaw(yaw), m_Pitch(pitch), m_Roll(roll)
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


void Camera::DefineView()
{
	m_ViewMatrix = LookAt(m_Position, m_Position + m_Front, m_Up);
	std::cout << m_ViewMatrix;
	system("CLS");
	m_Shader.Enable();
	m_Shader.SetUniformMatrix<4>("u_View", 1, GL_TRUE, m_ViewMatrix.a);
	m_Shader.Disable();
}

/*void Camera::CalcuateForwardAndUp(float yaw, float pitch, float roll)
{
	m_Yaw = yaw; 
	m_Pitch = pitch; 
	m_Roll = roll;

	//LookForward Vector
	Vec3 tempYaw;
	tempYaw.x = sin(m_Yaw);
	tempYaw.z = cos(m_Yaw);

	Vec3 tempPitch;
	tempPitch.x = cos(m_Pitch);
	tempPitch.y = sin(m_Pitch);
	tempPitch.z = cos(m_Pitch);

	m_LookForward.x = tempPitch.x * tempYaw.x;
	m_LookForward.y = tempPitch.y;
	m_LookForward.z = tempPitch.z * tempYaw.z;

	m_LookForward = m_LookForward.Normalise();

	//LookUp Vector
	Vec3 tempLookUp;
	tempLookUp.x = m_LookForward.x;
	tempLookUp.y = sin(m_Pitch + ((float)pi / 2)) * m_LookForward.y;
	tempLookUp.z = cos(m_Pitch + ((float)pi / 2)) * m_LookForward.z;

	tempLookUp.x = sin(m_Roll);
	tempLookUp.y = cos(m_Roll);

	m_LookUp = m_LookUp.Normalise(tempLookUp);
	m_LookRight = m_LookRight.Normalise(m_LookUp.Cross(m_LookForward));

	DefineView();
}*/

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
	Vec3 m_Position = camPos;
	Vec3 m_Front = Vec3::Normalise(camPos - camTarget);
	Vec3 m_Right = Vec3::Normalise(Vec3::Cross(m_Up, m_Front));
	Vec3 m_Up = Vec3::Normalise(Vec3::Cross(m_Right, m_Front));

	Mat4 output(Vec4(m_Right.x, m_Right.y, m_Right.z, 0),
		Vec4(m_Up.x, m_Up.y, m_Up.z, 0),
		Vec4(m_Front.x, m_Front.y, m_Front.z, 0),
		Vec4(0, 0, 0, 1));
	output * Mat4::Translation(Vec3(-m_Position.x, -m_Position.y, -m_Position.z));
	return output;
}
