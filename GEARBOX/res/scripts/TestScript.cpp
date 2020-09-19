#include "Scene/INativeScript.h"

using namespace gear;
using namespace scene;

class TestScript : public INativeScript
{
public:
	TestScript() = default;
	~TestScript() = default;

	bool first = true;
	mars::Vec3 initPos;

	void OnCreate()
	{
	}

	void OnDestroy()
	{
		CameraComponent* cameraComponent = GetCameraComponent();
		if (!cameraComponent)
			return;

		cameraComponent->GetCreateInfo().transform.translation = initPos;
	}

	void OnUpdate(float deltaTime)
	{
		CameraComponent* cameraComponent = GetCameraComponent();
		if (!cameraComponent)
			return;

		mars::Vec3& pos = cameraComponent->GetCreateInfo().transform.translation;
		if (first)
		{
			initPos = pos;
			first = false;
		}
		pos.z += 0.5f * deltaTime;
	}
};

GEAR_LOAD_SCRIPT(TestScript);
GEAR_UNLOAD_SCRIPT(TestScript);