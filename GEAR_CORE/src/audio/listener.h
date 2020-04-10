#pragma once

#include "gear_common.h"
#include "mars.h"
#include "graphics/OBJECTS/camera.h"

namespace GEAR {
namespace AUDIO {
class Listener
{
private:
	ALCdevice* m_Device;
	ALCcontext* m_Context;
	const GEAR::GRAPHICS::OBJECTS::Camera& m_Camera;

	float m_ListenerPosition[3];
	float m_ListenerVelocity[3];
	float m_ListenerOrientation[6];

public:
	Listener(const GEAR::GRAPHICS::OBJECTS::Camera& camera);
	~Listener();

	void SetVolume(float value);
	void UpdateListenerPosVelOri();
};
}
}

