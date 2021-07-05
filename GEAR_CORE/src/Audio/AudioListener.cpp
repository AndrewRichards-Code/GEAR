#include "gear_core_common.h"
#include "AudioListener.h"

using namespace gear;
using namespace audio;

AudioListener::AudioListener(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_ALICI.audioAPI = m_CI.audioAPI;
	m_ALICI.endPointDevice = m_CI.endPointDevice;
	m_ALI = CreateRef<AudioListenerInterface>(&m_ALICI);
}


AudioListener::~AudioListener()
{
}


/*void AudioListener::UpdateListener(const objects::Transform& transform)
{
	m_ListenerPosition[0] = transform.translation.x;
	m_ListenerPosition[1] = transform.translation.y;
	m_ListenerPosition[2] = transform.translation.z;

	m_ListenerVelocity[0] = 0.0f;
	m_ListenerVelocity[1] = 0.0f;
	m_ListenerVelocity[2] = 0.0f;

	m_ListenerOrientation[0] = transfrom.m_Direction.x;
	m_ListenerOrientation[1] = transfrom.m_Direction.y;
	m_ListenerOrientation[2] = transfrom.m_Direction.z;
	m_ListenerOrientation[3] = transfrom.m_Up.x;
	m_ListenerOrientation[4] = transfrom.m_Up.y;
	m_ListenerOrientation[5] = transfrom.m_Up.z;

	alListenerfv(AL_POSITION, m_ListenerPosition);
	alListenerfv(AL_VELOCITY, m_ListenerVelocity);
	alListenerfv(AL_ORIENTATION, m_ListenerOrientation);
}*/
