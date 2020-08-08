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
	auto toVec3 = [](const Vec4& vec) -> Vec3
	{
		return Vec3(vec.x, vec.y, vec.z);
	};

	m_Direction	= toVec3(m_CI.orientation * Vec4(0, 0, -1, 0));
	m_Up		= toVec3(m_CI.orientation * Vec4(0, +1, 0, 0));
	m_Right		= toVec3(m_CI.orientation * Vec4(+1, 0, 0, 0));

	m_UB->viewMatrix = m_CI.orientation * Mat4::Translation(-m_CI.position);
}


void Camera::SetPosition()
{
	m_UB->position = Vec4(m_CI.position, 1.0f);
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