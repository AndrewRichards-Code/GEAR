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
		SetHDRInfo();
	}
	if (TransformHasChanged(transform))
	{
		DefineView(transform);
		SetPosition(transform);
	}
	if (m_UpdateGPU)
	{
		m_CameraUB->SubmitData();
		m_HDRInfoUB->SubmitData();
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
		newHash ^= core::GetHash(CI.orthographicsParams.left);
		newHash ^= core::GetHash(CI.orthographicsParams.right);
		newHash ^= core::GetHash(CI.orthographicsParams.bottom);
		newHash ^= core::GetHash(CI.orthographicsParams.top);
		newHash ^= core::GetHash(CI.orthographicsParams.near);
		newHash ^= core::GetHash(CI.orthographicsParams.far);
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
	newHash ^= core::GetHash(CI.flipX);
	newHash ^= core::GetHash(CI.flipY);
	newHash ^= core::GetHash(CI.hdrSettings.exposure);
	newHash ^= core::GetHash(CI.hdrSettings.gammaSpace);
	return CompareCreateInfoHash(newHash);
}

void Camera::DefineProjection()
{
	if (m_CI.projectionType == ProjectionType::ORTHOGRAPHIC)
	{
		OrthographicParameters& op = m_CI.orthographicsParams;
		m_CameraUB->proj= float4x4::Orthographic(op.left, op.right, op.bottom, op.top, op.near, op.far);
	}
	else if (m_CI.projectionType == ProjectionType::PERSPECTIVE)
	{
		PerspectiveParameters& pp = m_CI.perspectiveParams;
		m_CameraUB->proj = float4x4::Perspective(pp.horizonalFOV, pp.aspectRatio, pp.zNear, pp.zFar);
	}
	else
	{
		GEAR_ASSERT(ErrorCode::OBJECTS | ErrorCode::INVALID_VALUE, "Unknown projection type.");
	}
	if (m_CI.flipX)
		m_CameraUB->proj.a *= -1;
	if (m_CI.flipY)
		m_CameraUB->proj.f *= -1;

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

void Camera::SetHDRInfo()
{
	m_HDRInfoUB->exposure = m_CI.hdrSettings.exposure;
	m_HDRInfoUB->gammaSpace = static_cast<uint32_t>(m_CI.hdrSettings.gammaSpace);
}

void Camera::InitialiseUB()
{
	float zero[sizeof(CameraUB)] = { 0 };
	Uniformbuffer<CameraUB>::CreateInfo ubCI;
	ubCI.debugName = "GEAR_CORE_Camera_CameraUB: " + m_CI.debugName;
	ubCI.device = m_CI.device;
	ubCI.data = zero;
	m_CameraUB = CreateRef<Uniformbuffer<CameraUB>>(&ubCI);

	float zero2[sizeof(HDRInfoUB)] = { 0 };
	Uniformbuffer<HDRInfoUB>::CreateInfo ubCI2;
	ubCI2.debugName = "GEAR_CORE_Camera_HDRInfo: " + m_CI.debugName;
	ubCI2.device = m_CI.device;
	ubCI2.data = zero2;
	m_HDRInfoUB = CreateRef<Uniformbuffer<HDRInfoUB>>(&ubCI2);
}