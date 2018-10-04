#include "audiosource.h"

using namespace GEAR;
using namespace AUDIO;

AudioSource::AudioSource(const char* filepath, const ARM::Vec3& position, const ARM::Vec3& direction)
 :m_FilePath(filepath), m_Position(position), m_Direction(direction)
{
	m_WavData = FileUtils::load_wav(m_FilePath);
	if (m_WavData.m_Channels == 1)
	{
		if (m_WavData.m_BitsPerSample == 8)
		{
			m_Format = AL_FORMAT_MONO8;
		}
		else {
			m_Format = AL_FORMAT_MONO16;
		}
	}
	else 
	{
		if (m_WavData.m_BitsPerSample == 8)
		{
			m_Format = AL_FORMAT_STEREO8;
		}
		else {
			m_Format = AL_FORMAT_STEREO16;
		}
	}
	alGenBuffers(1, &m_BufferID);
	alBufferData(m_BufferID, m_Format, m_WavData.m_Data, m_WavData.m_Size, m_WavData.m_SampleRate);

	alGenSources(1, &m_SourceID);
	alSourcef(m_SourceID, AL_GAIN, 1);
	alSourcef(m_SourceID, AL_PITCH, 1);
	alSourcei(m_SourceID, AL_LOOPING, AL_FALSE);
	
	UpdateSourcePosVelOri();
	Bind();
}

AudioSource::~AudioSource()
{
	alDeleteBuffers(1, &m_BufferID);
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

void AudioSource::Bind()
{
	alSourcei(m_SourceID, AL_BUFFER, m_BufferID);
}

void AudioSource::Unbind()
{
	alSourcei(m_SourceID, AL_BUFFER, 0);
}

void AudioSource::Play()
{
	alSourcePlay(m_SourceID);
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
	alGetSourcei(m_SourceID, AL_SOURCE_STATE, &m_IsLopped);
	if(m_IsLopped == AL_FALSE)
	{
		alSourcei(m_SourceID, AL_LOOPING, AL_TRUE);
	}
	else if (m_IsLopped == AL_TRUE)
	{
		alSourcei(m_SourceID, AL_LOOPING, AL_FALSE);
	}
}

void AudioSource::SetPitch(float value)
{
	alSourcef(m_SourceID, AL_PITCH, value);
}

void AudioSource::SetVolume(float value)
{
	alSourcef(m_SourceID, AL_GAIN, value);
}
