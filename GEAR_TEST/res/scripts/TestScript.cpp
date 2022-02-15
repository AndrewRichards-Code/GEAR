#include "Scene/INativeScript.h"

using namespace gear;
using namespace scene;

class TestScript : public INativeScript
{
public:
	TestScript() = default;
	~TestScript() = default;

	void OnCreate()
	{
	}

	void OnDestroy()
	{
	}

	void OnUpdate(float deltaTime)
	{
		CameraComponent* cameraComponent = GetCameraComponent();
		if (!cameraComponent)
			return;

		mars::float3& pos = cameraComponent->GetCreateInfo().transform.translation;
		pos.z += 1.0f * deltaTime;
	}
};

GEAR_LOAD_SCRIPT(TestScript);
GEAR_UNLOAD_SCRIPT(TestScript);