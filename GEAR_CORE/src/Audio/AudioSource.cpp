#include "gear_core_common.h"
#include "AudioSource.h"

using namespace gear;
using namespace audio;

AudioSource::AudioSource(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_ASICI.filepath = m_CI.filepath;
	m_ASICI.pAudioListener = m_CI.pAudioListener->GetAudioListenerInterface();
	m_ASI = CreateRef<AudioSourceInterface>(&m_ASICI);
}

AudioSource::~AudioSource()
{
	m_AudioStreamThread.join();
}

void AudioSource::SetPitch(float value)
{
	m_ASI->SetPitch(value);
}

void AudioSource::SetVolume(float value)
{
	m_ASI->SetVolume(value);
}

void AudioSource::Stream()
{
	auto AudioStream = [&]()
	{
		while (true)
		{
			m_ASI->Stream();
			
			float bufferTimeInS = float(m_ASI->GetWavData()->buffer1.size()) / float(m_ASI->GetWavData()->sampleRate * m_ASI->GetWavData()->blockAlign);
			std::chrono::duration<float> duration(bufferTimeInS / 4.0f);
			std::this_thread::sleep_for(duration);
		}
	};

	m_AudioStreamThread = std::thread(AudioStream);
}

void AudioSource::Loop()
{
	if(m_Looped == false)
	{
		m_ASI->Unloop();
	}
	else if (m_Looped == true)
	{
		m_ASI->Loop();
	}
}