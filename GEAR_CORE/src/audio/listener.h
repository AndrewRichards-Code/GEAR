#pragma once

#include "AL/al.h"
#include "AL/alc.h"
#include "../maths/ARMLib.h"
#include "../graphics/camera.h"

namespace GEAR {
namespace AUDIO {
class Listener
{
private:
	ALCdevice* m_Device;
	ALCcontext* m_Context;
	const GEAR::GRAPHICS::Camera& m_Camera;

	float m_ListenerPosition[3];
	float m_ListenerVelocity[3];
	float m_ListenerOrientation[6];

public:
	Listener(const GEAR::GRAPHICS::Camera& camera);
	~Listener();

	void SetVolume(float value);
	void UpdateListenerPosVelOri();
};
}
}
