#include "listener.h"

using namespace GEAR;
using namespace AUDIO;

Listener::Listener(const GEAR::GRAPHICS::OBJECTS::Camera& camera)
	: m_Camera(camera)
{
	m_Device = alcOpenDevice(NULL);
	if (!m_Device)
	{
		std::cout << "ERROR: GEAR::AUDIO::Listener: Failed to initialise OpenAL!" << std::endl;
	}
	
	m_Context = alcCreateContext(m_Device, NULL);
	if (!m_Context)
	{
		std::cout << "ERROR: GEAR::AUDIO::Listener: Failed to create context!" << std::endl;
	}
	alcMakeContextCurrent(m_Context);
	UpdateListenerPosVelOri();
	SetVolume(1);
}


Listener::~Listener()
{
	alcMakeContextCurrent(m_Context);
	alcDestroyContext(m_Context);
	alcCloseDevice(m_Device);
}

void Listener::SetVolume(float value)
{
	alListenerf(AL_GAIN, value);
}

void Listener::UpdateListenerPosVelOri()
{
	m_ListenerPosition[0] = m_Camera.m_Position.x;
	m_ListenerPosition[1] = m_Camera.m_Position.y;
	m_ListenerPosition[2] = m_Camera.m_Position.z;

	m_ListenerVelocity[0] = 0.0f;
	m_ListenerVelocity[1] = 0.0f;
	m_ListenerVelocity[2] = 0.0f;

	m_ListenerOrientation[0] = m_Camera.m_Forward.x;
	m_ListenerOrientation[1] = m_Camera.m_Forward.y;
	m_ListenerOrientation[2] = m_Camera.m_Forward.z;
	m_ListenerOrientation[3] = m_Camera.m_Up.x;
	m_ListenerOrientation[4] = m_Camera.m_Up.y;
	m_ListenerOrientation[5] = m_Camera.m_Up.z;

	alListenerfv(AL_POSITION, m_ListenerPosition);
	alListenerfv(AL_VELOCITY, m_ListenerVelocity);
	alListenerfv(AL_ORIENTATION, m_ListenerOrientation);
}
