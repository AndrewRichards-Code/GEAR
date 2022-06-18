#include "gear_core_common.h"
#include "Graphics/DebugRender.h"
#include "Graphics/AllocatorManager.h"
#include "Objects/Camera.h"

using namespace gear;
using namespace graphics;

void* DebugRender::s_Device = nullptr;
Ref<objects::Camera> DebugRender::s_Camera = nullptr;

Ref<objects::Camera>& DebugRender::GetCamera()
{
	using namespace objects;
	if (!s_Camera && s_Device)
	{
		Camera::CreateInfo cameraCI;
		cameraCI.debugName = "GEAR_CORE_Camera: GEAR_CORE_DebugRender";
		cameraCI.device = s_Device;
		cameraCI.projectionType = Camera::ProjectionType::PERSPECTIVE;
		cameraCI.perspectiveParams = { mars::DegToRad(120.0), 1.0f, 0.001f, 1.0f };
		cameraCI.flipX = false;
		cameraCI.flipY = false;
		s_Camera = CreateRef<Camera>(&cameraCI);
	}

	return s_Camera;
}
