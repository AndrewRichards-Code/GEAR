#include "gearbox_common.h"
#include "CameraComponentUI.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace gearbox;
using namespace componentui;

using namespace mars;

void gearbox::componentui::DrawCameraComponentUI(gear::scene::Entity entity, float screenRatio)
{
	Ref<Camera>& camera = entity.GetComponent<CameraComponent>().camera;
	Camera::CreateInfo& CI = camera->m_CI;

	DrawDropDownMenu("Projection Type", { "Perspective", "Orthographic" }, CI.projectionType);

	if (CI.projectionType == Camera::ProjectionType::PERSPECTIVE)
	{
		static Camera::PerspectiveParameters pp = { DegToRad(90.0), screenRatio, 0.001f, 3000.0f };

		double fovDeg = RadToDeg(pp.horizonalFOV);
		DrawDouble("FOV", fovDeg, 0.0, 180.0);
		pp.horizonalFOV = DegToRad(fovDeg);
		pp.aspectRatio = screenRatio;
		DrawFloat("Near", pp.zNear,  0.0, pp.zFar);
		DrawFloat("Far", pp.zFar, pp.zNear);

		CI.perspectiveParams = pp;
	}
	else
	{
		static Camera::OrthographicParameters op = { -1.0f, 1.0f, -screenRatio, +screenRatio, 0.001f, 3000.0f };

		op.left = -1.0f;
		op.right = +1.0f;
		op.bottom = -screenRatio;
		op.top = +screenRatio;
		DrawFloat("Near", op.near, 0.0f, op.far);
		DrawFloat("Far", op.far, op.near);

		CI.orthographicsParams = op;
	}
	DrawCheckbox("Flip X", CI.flipX);
	DrawCheckbox("Flip Y", CI.flipY);

	if (ImGui::IsWindowFocused())
		CI.transform = entity.GetComponent<TransformComponent>().transform; //Properties panel focused
	else
		entity.GetComponent<TransformComponent>().transform = CI.transform; //Any other panel focused , i.e. ViewportPanel

	camera->Update();
}

void gearbox::componentui::AddCameraComponent(gear::scene::Entity entity, float screenRatio, void* device)
{
	if (!entity.HasComponent<CameraComponent>())
	{
		Camera::CreateInfo cameraCI;
		cameraCI.debugName = entity.GetComponent<NameComponent>();
		cameraCI.device = device;
		cameraCI.transform = entity.GetComponent<TransformComponent>();
		cameraCI.projectionType = Camera::ProjectionType::PERSPECTIVE;
		cameraCI.perspectiveParams = { DegToRad(90.0), screenRatio, 0.001f, 3000.0f };
		cameraCI.flipX = false;
		cameraCI.flipY = false;
		entity.AddComponent<CameraComponent>(&cameraCI);

		entity.GetComponent<TransformComponent>().transform = cameraCI.transform;
	}
}
