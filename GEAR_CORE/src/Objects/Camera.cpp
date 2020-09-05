#include "gear_core_common.h"
#include "Camera.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

Camera::Camera(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

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
		GEAR_ASSERT(core::Log::Level::ERROR, core::Log::ErrorCode::OBJECTS| core::Log::ErrorCode::INVALID_VALUE, "The parameters for DefinieProjection() don't match the projection type.");
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
	const Mat4& orientation = m_CI.transform.orientation.ToMat4();

	m_Direction	= Vec3(orientation * Vec4(0, 0, -1, 0));
	m_Up		= Vec3(orientation * Vec4(0, +1, 0, 0));
	m_Right		= Vec3(orientation * Vec4(+1, 0, 0, 0));

	m_UB->viewMatrix = orientation * Mat4::Translation(-m_CI.transform.translation);
}


void Camera::SetPosition()
{
	m_UB->position = Vec4(m_CI.transform.translation, 1.0f);
}

void Camera::InitialiseUB()
{
	float zero[sizeof(CameraUB)] = { 0 };
	
	Uniformbuffer<CameraUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Camera: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_UB = gear::CreateRef<Uniformbuffer<CameraUB>>(&ubCI);
}