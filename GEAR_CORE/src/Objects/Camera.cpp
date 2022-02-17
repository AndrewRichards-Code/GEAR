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
	Update(Transform());
}

Camera::~Camera()
{
}

void Camera::Update(const Transform& transform)
{
	DefineProjection();
	DefineView(transform);
	SetPosition(transform);
	m_UB->SubmitData();
}

void Camera::DefineProjection()
{
	if (m_CI.projectionType == ProjectionType::ORTHOGRAPHIC)
	{
		OrthographicParameters& op = m_CI.orthographicsParams;
		m_UB->proj= float4x4::Orthographic(op.left, op.right, op.bottom, op.top, op.near, op.far);
	}
	else if (m_CI.projectionType == ProjectionType::PERSPECTIVE)
	{
		PerspectiveParameters& pp = m_CI.perspectiveParams;
		m_UB->proj = float4x4::Perspective(pp.horizonalFOV, pp.aspectRatio, pp.zNear, pp.zFar);
	}
	else
	{
		GEAR_ASSERT(ErrorCode::OBJECTS | ErrorCode::INVALID_VALUE, "Unknown projection type.");
		throw;
	}
	if (m_CI.flipX)
		m_UB->proj.a *= -1;
	if (m_CI.flipY)
		m_UB->proj.f *= -1;

	if (miru::crossplatform::GraphicsAPI::IsVulkan())
		m_UB->proj.f *= -1;
}

void Camera::DefineView(const Transform& transform)
{
	const float4x4& orientation = transform.orientation.ToRotationMatrix4<float>();

	m_Direction	= -float3(orientation * float4(0, 0, 1, 0));
	m_Up		= +float3(orientation * float4(0, 1, 0, 0));
	m_Right		= +float3(orientation * float4(1, 0, 0, 0));

	m_UB->view = TransformToMatrix4(transform).Inverse();
}


void Camera::SetPosition(const Transform& transform)
{
	m_UB->cameraPosition = float4(transform.translation, 1.0f);
}

void Camera::InitialiseUB()
{
	float zero[sizeof(CameraUB)] = { 0 };
	Uniformbuffer<CameraUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Camera_CameraUB: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_UB = CreateRef<Uniformbuffer<CameraUB>>(&ubCI);
}