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
	if (CreateInfoHasChanged(&m_CI))
	{
		DefineProjection();
	}
	if (TransformHasChanged(transform))
	{
		DefineView(transform);
		SetPosition(transform);
	}
	if (m_UpdateGPU)
	{
		m_CameraUB->SubmitData();
	}
}

float4x4 Camera::GetCubemapFaceViewMatrix(uint32_t faceIndex, const float3& translation)
{
	float4x4 view;
	float3 _translation = translation;

	switch (faceIndex)
	{
	default:
		view = float4x4();
	
	case 0: // POSITIVE_X
		view = float4x4(
		+0.0f, +0.0f, -1.0f, +0.0f,
		+0.0f, +1.0f, +0.0f, +0.0f,
		+1.0f, +0.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, +0.0f, +1.0f);
		break;

	case 1: // NEGATIVE_X
		view = float4x4(
		+0.0f, +0.0f, +1.0f, +0.0f,
		+0.0f, +1.0f, +0.0f, +0.0f,
		-1.0f, +0.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, +0.0f, +1.0f);
		break;

	case 2: // POSITIVE_Y
		view = float4x4(
		+1.0f, +0.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, -1.0f, +0.0f,
		+0.0f, +1.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, +0.0f, +1.0f);
		break;
	
	case 3: // NEGATIVE_Y
		view = float4x4(
		+1.0f, +0.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, +1.0f, +0.0f,
		+0.0f, -1.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, +0.0f, +1.0f);
		break;

	case 4: // POSITIVE_Z
		view = float4x4(
		+1.0f, +0.0f, +0.0f, +0.0f,
		+0.0f, +1.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, +1.0f, +0.0f,
		+0.0f, +0.0f, +0.0f, +1.0f);
		break;

	case 5: // NEGATIVE_Z
		view = float4x4(
		-1.0f, +0.0f, +0.0f, +0.0f,
		+0.0f, +1.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, -1.0f, +0.0f,
		+0.0f, +0.0f, +0.0f, +1.0f);
		break;
	}
	return view * float4x4::Translation(-(_translation));
}

bool Camera::CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo)
{
	const CreateInfo& CI = *reinterpret_cast<const CreateInfo*>(pCreateInfo);
	uint64_t newHash = 0;
	newHash ^= core::GetHash(CI.projectionType);
	if (m_CI.projectionType == ProjectionType::ORTHOGRAPHIC)
	{
		newHash ^= core::GetHash(CI.orthographicParams.left);
		newHash ^= core::GetHash(CI.orthographicParams.right);
		newHash ^= core::GetHash(CI.orthographicParams.bottom);
		newHash ^= core::GetHash(CI.orthographicParams.top);
		newHash ^= core::GetHash(CI.orthographicParams.near);
		newHash ^= core::GetHash(CI.orthographicParams.far);
	}
	else if(m_CI.projectionType == ProjectionType::PERSPECTIVE)
	{
		newHash ^= core::GetHash(CI.perspectiveParams.horizonalFOV);
		newHash ^= core::GetHash(CI.perspectiveParams.aspectRatio);
		newHash ^= core::GetHash(CI.perspectiveParams.zNear);
		newHash ^= core::GetHash(CI.perspectiveParams.zFar);
	}
	else
	{
		GEAR_ASSERT(ErrorCode::OBJECTS | ErrorCode::INVALID_VALUE, "Unknown projection type.");
	}
	return CompareCreateInfoHash(newHash);
}

void Camera::DefineProjection()
{
	if (m_CI.projectionType == ProjectionType::ORTHOGRAPHIC)
	{
		OrthographicParameters& op = m_CI.orthographicParams;
		m_CameraUB->proj= float4x4::Orthographic(op.left, op.right, op.bottom, op.top, op.near, op.far, true, false);
	}
	else if (m_CI.projectionType == ProjectionType::PERSPECTIVE)
	{
		PerspectiveParameters& pp = m_CI.perspectiveParams;
		m_CameraUB->proj = float4x4::Perspective(pp.horizonalFOV, pp.aspectRatio, pp.zNear, pp.zFar, true, false);
	}
	else
	{
		GEAR_ASSERT(ErrorCode::OBJECTS | ErrorCode::INVALID_VALUE, "Unknown projection type.");
	}

	if (miru::base::GraphicsAPI::IsVulkan())
		m_CameraUB->proj.f *= -1;
}

void Camera::DefineView(const Transform& transform)
{
	const float4x4& orientation = transform.orientation.ToRotationMatrix4<float>();

	m_Direction	= -float3(orientation * float4(0, 0, 1, 0));
	m_Up		= +float3(orientation * float4(0, 1, 0, 0));
	m_Right		= +float3(orientation * float4(1, 0, 0, 0));

	m_CameraUB->view = TransformToMatrix4(transform).Inverse();
}


void Camera::SetPosition(const Transform& transform)
{
	m_CameraUB->position = float4(transform.translation, +1.0f);
}

void Camera::InitialiseUB()
{
	float zero[sizeof(CameraUB)] = { 0 };
	Uniformbuffer<CameraUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Camera_CameraUB: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_CameraUB = CreateRef<Uniformbuffer<CameraUB>>(&ubCI);
}