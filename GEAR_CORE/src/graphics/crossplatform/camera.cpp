#include "camera.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace CROSSPLATFORM;
using namespace ARM;

bool Camera::s_InitialiseUBO = false;

Camera::Camera(int projType, const ARM::Vec3& position, const ARM::Vec3& forward, const ARM::Vec3 up)
	:m_ProjectionType(projType), m_Position(position), m_Forward(forward), m_Up(up)
{
	InitialiseUBO();

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
	m_CameraUBO.m_Position = Vec4(m_Position, 1.0f);
	OPENGL::BufferManager::UpdateUBO(0, (const float*)&m_CameraUBO.m_Position.x, sizeof(Vec4), offsetof(CameraUBO, m_Position));
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
	m_CameraUBO.m_ViewMatrix =
		Mat4::Rotation(m_Pitch, m_xAxis) *
		Mat4::Rotation(m_Yaw, m_yAxis) *
		Mat4::Rotation(m_Roll, m_zAxis) *
		Mat4::Translation(Vec3(-m_Position.x, -m_Position.y, -m_Position.z));

	/*CalculateRight();
	//std::cout <<"Forward: "<< m_Forward << ", Right: " << m_Right << ", Up: " << m_Up << std::endl;
	//system("CLS");
	Quat x((float)m_Pitch, m_Right);
	Quat y((float)m_Yaw, m_yAxis);
	Quat orientation = y * x;
	orientation.Normalise();
	Mat4 rotation = Quat::ToMat4(orientation);
	Mat4 translation = Mat4::Translation(Vec3(-m_Position.x, -m_Position.y, -m_Position.z));
	m_CameraUBO.m_ViewMatrix = rotation * translation;*/

	/*Vec3 p(1, 0, 0);
	Quat q((float) pi / 4, Vec3(0, 1, 0));
	Quat qInv = q.Conjugate();

	Vec3 final = Quat::ToVec3((q * p) * qInv);
	Vec3 final2 = p.RotQuat(q);*/

	m_CameraUBO.m_ViewMatrix.Transpose();
	OPENGL::BufferManager::UpdateUBO(0, &m_CameraUBO.m_ViewMatrix.a, sizeof(Mat4), offsetof(CameraUBO, m_ViewMatrix));
}
void Camera::DefineView(const Mat4& viewMatrix)
{
	m_CameraUBO.m_ViewMatrix = viewMatrix * Mat4::Translation(Vec3(-m_Position.x, -m_Position.y, -m_Position.z));
	m_CameraUBO.m_ViewMatrix.Transpose();
	OPENGL::BufferManager::UpdateUBO(0, &m_CameraUBO.m_ViewMatrix.a, sizeof(Mat4), offsetof(CameraUBO, m_ViewMatrix));
}
void Camera::DefineProjection(float left, float right, float bottom, float top, float near, float far)
{
	if (m_ProjectionType == GEAR_CAMERA_ORTHOGRAPHIC)
	{
		m_CameraUBO.m_ProjectionMatrix = Mat4::Orthographic(left, right, bottom, top, near, far);
		m_CameraUBO.m_ProjectionMatrix.Transpose();

		OPENGL::BufferManager::UpdateUBO(0, &m_CameraUBO.m_ProjectionMatrix.a, sizeof(Mat4), offsetof(CameraUBO, m_ProjectionMatrix));
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::CROSSPLATFORM::Camera: The parameters for DefinieProjection() don't match the projection type." << std::endl;
		throw;
	}
}
void Camera::DefineProjection(double fov, float aspectRatio, float zNear, float zFar, bool flipX, bool flipY)
{
	if (m_ProjectionType == GEAR_CAMERA_PERSPECTIVE)
	{
		m_CameraUBO.m_ProjectionMatrix = Mat4::Perspective(fov, aspectRatio, zNear, zFar);
		if (flipX)
			m_CameraUBO.m_ProjectionMatrix.a *= -1;
		if (flipY)
			m_CameraUBO.m_ProjectionMatrix.f *= -1;
		m_CameraUBO.m_ProjectionMatrix.Transpose();

		OPENGL::BufferManager::UpdateUBO(0, &m_CameraUBO.m_ProjectionMatrix.a, sizeof(Mat4), offsetof(CameraUBO, m_ProjectionMatrix));
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::CROSSPLATFORM::Camera: The parameters for DefinieProjection() don't match the projection type." << std::endl;
		throw;
	}
}

//private:
void Camera::CalculateRight()
{
	m_Right = Vec3::Normalise(Vec3::Cross(m_Forward, m_Up));
}
void Camera::CalculateUp()
{
	m_Up = Vec3::Normalise(Vec3::Cross(m_Forward, m_Right));
}

void Camera::InitialiseUBO()
{
	if (s_InitialiseUBO == false)
	{
		OPENGL::BufferManager::AddUBO(sizeof(CameraUBO), 0);
		s_InitialiseUBO = true;

		const float zero[sizeof(CameraUBO)] = { 0 };
		OPENGL::BufferManager::UpdateUBO(0, zero, sizeof(CameraUBO), 0);
	}
}