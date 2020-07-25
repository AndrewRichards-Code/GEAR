#pragma once

#include "gear_core_common.h"
#include "mars.h"
#include "Objects/Camera.h"

namespace gear {
namespace audio {
class Listener
{
private:
	ALCdevice* m_Device;
	ALCcontext* m_Context;
	const gear::objects::Camera& m_Camera;

	float m_ListenerPosition[3];
	float m_ListenerVelocity[3];
	float m_ListenerOrientation[6];

public:
	Listener(const gear::objects::Camera& camera);
	~Listener();

	void SetVolume(float value);
	void UpdateListenerPosVelOri();
};
}
}

