#include "audiosource.h"

using namespace gear;
using namespace audio;

AudioSource::AudioSource(const char* filepath, const mars::Vec3& position, const mars::Vec3& direction)
	:m_FilePath(filepath), m_Position(position), m_Direction(direction)
{
	m_WavData = FileUtils::stream_wav(m_FilePath);
	if (m_WavData->m_Channels == 1)
	{
		if (m_WavData->m_BitsPerSample == 8)
		{
			m_Format = AL_FORMAT_MONO8;
		}
		else {
			m_Format = AL_FORMAT_MONO16;
		}
	}
	else 
	{
		if (m_WavData->m_BitsPerSample == 8)
		{
			m_Format = AL_FORMAT_STEREO8;
		}
		else {
			m_Format = AL_FORMAT_STEREO16;
		}
	}
	alGenBuffers(2, &m_BufferID[0]);
	alGenSources(1, &m_SourceID);

	Loop(); //Used to catch the first two buffers, in case the track is looped.
	SubmitBuffer();
	SubmitBuffer();
	Loop(); //Same as above.

	alSourceQueueBuffers(m_SourceID, 2, &m_BufferID[0]);
	
	alSourcef(m_SourceID, AL_GAIN, 1);
	alSourcef(m_SourceID, AL_PITCH, 1);
	alSourcei(m_SourceID, AL_LOOPING, AL_FALSE);
	
	UpdateSourcePosVelOri();
}

AudioSource::~AudioSource()
{
	alDeleteBuffers(2, &m_BufferID[0]);
	alDeleteSources(1, &m_SourceID);
}

void AudioSource::UpdateSourcePosVelOri()
{
	alSource3f(m_SourceID, AL_POSITION, m_Position.x, m_Position.y, m_Position.z);
	alSource3f(m_SourceID, AL_VELOCITY, m_Velocity.x, m_Velocity.y, m_Velocity.z);
	alSource3f(m_SourceID, AL_DIRECTION, m_Direction.x, m_Direction.y, m_Direction.z);
}

void AudioSource::DefineConeParameters(float outerGain, double innerAngle, double outerAngle)
{
	alSourcef(m_SourceID, AL_CONE_OUTER_GAIN, outerGain);
	alSourcef(m_SourceID, AL_CONE_INNER_ANGLE, static_cast<float>(innerAngle));
	alSourcef(m_SourceID, AL_CONE_OUTER_ANGLE, static_cast<float>(outerAngle));
}


void AudioSource::SetPitch(float value) //By semitones (-6.0f < value < 6.0f).
{
	if (value > 6.0f || value < -6.0f)
		std::cout << "ERROR: GEAR::AUDIO::AudioSource::SetPitch: Input value out of range! Pitch has not been changed!" << std::endl;
	else
		alSourcef(m_SourceID, AL_PITCH, pow(2.0f, (value / 12.0f)));
}

void AudioSource::SetVolume(float value) //By decibels.
{
	float dB = pow(10.0f, (value / 20.0f));
	if (dB < 0.0f)
		std::cout << "ERROR: GEAR::AUDIO::AudioSource::SetVolume: Calcualted value out of range! Volume has not been changed!" << std::endl;
	else
		alSourcef(m_SourceID, AL_GAIN, dB);
}

void AudioSource::Stream()
{
	if (!m_Ended)
	{
		Play();
	}

	int processed;
	alGetSourcei(m_SourceID, AL_BUFFERS_PROCESSED, &processed);
	/*std::cout << "Buffers: " << processed << std::endl;
	std::cout << "Next Buffer: " << m_WavData->m_NextBuffer << std::endl;*/
	
	while (processed--)
	{
		switch (m_WavData->m_NextBuffer)
		{
		case 1:
			alSourceUnqueueBuffers(m_SourceID, 1, &m_BufferID[0]);
			SubmitBuffer();
			alSourceQueueBuffers(m_SourceID, 1, &m_BufferID[0]);
			break;
		case 2:
			alSourceUnqueueBuffers(m_SourceID, 1, &m_BufferID[1]);
			SubmitBuffer();
			alSourceQueueBuffers(m_SourceID, 1, &m_BufferID[1]);
			break;
		case 0:
			m_Ended = true;
			alSourceUnqueueBuffers(m_SourceID, 2, &m_BufferID[0]);
			Stop();
		}
	}
}

void AudioSource::Play()
{
	int playing;
	alGetSourcei(m_SourceID, AL_SOURCE_STATE, &playing);
	
	if (playing != AL_PLAYING)
		alSourcePlay(m_SourceID);

	/*switch (playing)
	{
	case 0x1011: std::cout << "AL_INITIAL" << std::endl; break;
	case 0x1012: std::cout << "AL_PLAYING" << std::endl; break;
	case 0x1013: std::cout << "AL_PAUSED"  << std::endl; break;
	case 0x1014: std::cout << "AL_STOPPED" << std::endl; break;
	}*/
}

void AudioSource::Stop()
{
	alSourceStop(m_SourceID);
}

void AudioSource::Pause()
{
	alSourcePause(m_SourceID);
}

void AudioSource::Loop()
{
	if(m_Looped == false)
	{
		m_WavData->m_LoopBufferQueue = true;
	}
	else if (m_Looped == true)
	{
		m_WavData->m_LoopBufferQueue = false;
	}
}
void AudioSource::SubmitBuffer()
{
	FileUtils::get_next_wav_block(*m_WavData);
	switch (m_WavData->m_NextBuffer)
	{
	case 1:
		alBufferData(m_BufferID[1], m_Format, m_WavData->m_Buffer2.data(), static_cast<ALsizei>(m_WavData->m_Buffer2.size()), m_WavData->m_SampleRate);
	case 2:
		alBufferData(m_BufferID[0], m_Format, m_WavData->m_Buffer1.data(), static_cast<ALsizei>(m_WavData->m_Buffer1.size()), m_WavData->m_SampleRate);
	}
}