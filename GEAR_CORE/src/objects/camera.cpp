#include "camera.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

Camera::Camera(void* device, int projType, const mars::Vec3& position, const mars::Vec3& forward, const mars::Vec3 up)
	:m_Device(device), m_ProjectionType(projType), m_Position(position), m_Forward(forward), m_Up(up)
{
	InitialiseUB();

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
	m_CameraUB.m_Position = Vec4(m_Position, 1.0f);
	m_UB->SubmitData((const float*)&m_CameraUB.m_ProjectionMatrix.a, sizeof(CameraUB));
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
	m_CameraUB.m_ViewMatrix =
		Mat4::Rotation(m_Pitch, m_xAxis) *
		Mat4::Rotation(m_Yaw, m_yAxis) *
		Mat4::Rotation(m_Roll, m_zAxis) *
		Mat4::Translation(Vec3(-m_Position.x, -m_Position.y, -m_Position.z));

	/*CalculateRight();
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

	m_UB->SubmitData((const float*)&m_CameraUB.m_ProjectionMatrix.a, sizeof(CameraUB));
}
void Camera::DefineView(const Mat4& viewMatrix)
{
	Mat4 translation = Mat4::Translation(Vec3(-m_Position.x, -m_Position.y, -m_Position.z));
	m_CameraUB.m_ViewMatrix = viewMatrix * translation;
	m_UB->SubmitData((const float*)&m_CameraUB.m_ProjectionMatrix.a, sizeof(CameraUB));
}
void Camera::DefineProjection(float left, float right, float bottom, float top, float near, float far)
{
	if (m_ProjectionType == GEAR_CAMERA_ORTHOGRAPHIC)
	{
		m_CameraUB.m_ProjectionMatrix = Mat4::Orthographic(left, right, bottom, top, near, far);
		m_UB->SubmitData((const float*)&m_CameraUB.m_ProjectionMatrix.a, sizeof(CameraUB));
	}
	else
	{
		GEAR_ASSERT(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_INVALID_VALUE, "ERROR: GEAR::OBJECTS::Camera: The parameters for DefinieProjection() don't match the projection type.");
		throw;
	}
}
void Camera::DefineProjection(double fov, float aspectRatio, float zNear, float zFar, bool flipX, bool flipY)
{
	if (m_ProjectionType == GEAR_CAMERA_PERSPECTIVE)
	{
		m_CameraUB.m_ProjectionMatrix = Mat4::Perspective(fov, aspectRatio, zNear, zFar);
		if (flipX)
			m_CameraUB.m_ProjectionMatrix.a *= -1;
		if (flipY)
			m_CameraUB.m_ProjectionMatrix.f *= -1;

		if (miru::GraphicsAPI::IsVulkan())
			m_CameraUB.m_ProjectionMatrix.f *= -1;

		m_UB->SubmitData((const float*)&m_CameraUB.m_ProjectionMatrix.a, sizeof(CameraUB));
	}
	else
	{
		GEAR_ASSERT(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_INVALID_VALUE, "ERROR: GEAR::OBJECTS::Camera: The parameters for DefinieProjection() don't match the projection type.");
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

void Camera::InitialiseUB()
{
	float zero[sizeof(CameraUB)] = { 0 };
	
	UniformBuffer::CreateInfo ubCI;
	ubCI.device = m_Device;
	ubCI.data = zero;
	ubCI.size = sizeof(CameraUB);
	m_UB = gear::CreateRef<UniformBuffer>(&ubCI);
}