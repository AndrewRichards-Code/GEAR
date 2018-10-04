#pragma once

#include "AL/al.h"
#include "../maths/ARMLib.h"
#include "../utils/fileutils.h"

namespace GEAR {
namespace AUDIO {
class AudioSource
{
private:
	const char* m_FilePath;
	unsigned int m_BufferID;
	unsigned int m_SourceID;

	FileUtils::WavData m_WavData;
	unsigned int m_Format;

	ARM::Vec3 m_Position;
	ARM::Vec3 m_Velocity = { 0.0f, 0.0f, 0.0f };
	ARM::Vec3 m_Direction;

	int m_IsLopped = AL_FALSE;
	
public:
	AudioSource(const char* filepath, const ARM::Vec3& position, const ARM::Vec3& direction);
	~AudioSource();

	void UpdateSourcePosVelOri();
	void DefineConeParameters(float outerGain, double innerAngle, double outerAngle);

	void Bind();
	void Unbind();

	void Play();
	void Stop();
	void Pause();
	void Loop();
	void SetPitch(float value);
	void SetVolume(float value);
};
}
}