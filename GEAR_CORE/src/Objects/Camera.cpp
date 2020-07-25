#include "gear_core_common.h"
#include "Camera.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

Camera::Camera(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	m_DebugName = std::string("GEAR_CORE_Camera: ") + m_CI.debugName;

	InitialiseUB();
	Update();
}

Camera::~Camera()
{
}

void Camera::Update()
{
	DefineProjection();
	DefineView();
	SetPosition();
	m_UB->SubmitData();
}

//private:

void Camera::DefineProjection()
{
	if (m_CI.projectionType == ProjectionType::ORTHOGRAPHIC)
	{
		OrthographicParameters& op = m_CI.orthographicsParams;
		m_UB->projectionMatrix = Mat4::Orthographic(op.left, op.right, op.bottom, op.top, op.near, op.far);
	}
	else if (m_CI.projectionType == ProjectionType::PERSPECTIVE)
	{
		PerspectiveParameters& pp = m_CI.perspectiveParams;
		m_UB->projectionMatrix = Mat4::Perspective(pp.horizonalFOV, pp.aspectRatio, pp.zNear, pp.zFar);
	}
	else
	{
		GEAR_ASSERT(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_INVALID_VALUE, "ERROR: GEAR::OBJECTS::Camera: The parameters for DefinieProjection() don't match the projection type.");
		throw;
	}
	if (m_CI.flipX)
		m_UB->projectionMatrix.a *= -1;
	if (m_CI.flipY)
		m_UB->projectionMatrix.f *= -1;

	if (miru::crossplatform::GraphicsAPI::IsVulkan())
		m_UB->projectionMatrix.f *= -1;
}

void Camera::DefineView()
{
	m_Direction	= mars::Vec3(0, 0, +1).RotateQuat(m_CI.orientation);
	m_Up		= mars::Vec3(0, +1, 0).RotateQuat(m_CI.orientation);
	m_Right		= mars::Vec3(+1, 0, 0).RotateQuat(m_CI.orientation);


	m_UB->viewMatrix = Quat::ToMat4(m_CI.orientation) * Mat4::Translation(-m_CI.position);
}


void Camera::SetPosition()
{
	m_UB->position = Vec4(m_CI.position, 1.0f);
}

void Camera::InitialiseUB()
{
	float zero[sizeof(CameraUB)] = { 0 };
	
	Uniformbuffer<CameraUB>::CreateInfo ubCI;
	ubCI.debugName = m_DebugName.c_str();
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_UB = gear::CreateRef<Uniformbuffer<CameraUB>>(&ubCI);
}