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
		objects::Transform& transform = GetEntity().GetComponent<TransformComponent>().transform;
		mars::float3& pos = transform.translation;
		pos.z += 1.0f * deltaTime;
	}
};

GEAR_LOAD_SCRIPT(TestScript);
GEAR_UNLOAD_SCRIPT(TestScript);