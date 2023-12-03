#include "Scene/NativeScript.h"

using namespace gear;
using namespace scene;

class TestScript : public NativeScript
{
public:
	TestScript() = default;
	~TestScript() = default;

	bool first = true;
	mars::float3 initPos;

	void OnCreate()
	{
	}

	void OnDestroy()
	{
		objects::Transform& transform = GetEntity().GetComponent<TransformComponent>().transform;
		transform.translation = initPos;
	}

	void OnUpdate(float deltaTime)
	{
		objects::Transform& transform = GetEntity().GetComponent<TransformComponent>().transform;
		mars::float3& pos = transform.translation;
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