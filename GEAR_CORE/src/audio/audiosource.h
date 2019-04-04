#pragma once

#include "AL/al.h"
#include "maths/ARMLib.h"
#include "utils/fileutils.h"

namespace GEAR {
namespace AUDIO {
class AudioSource
{
private:
	const char* m_FilePath;
	unsigned int m_BufferID[2];
	unsigned int m_SourceID;

	std::shared_ptr<FileUtils::WavData> m_WavData;
	unsigned int m_Format;

	ARM::Vec3 m_Position;
	ARM::Vec3 m_Velocity = { 0.0f, 0.0f, 0.0f };
	ARM::Vec3 m_Direction;

	bool m_Looped = false;
	bool m_Ended = false;
	
public:
	AudioSource(const char* filepath, const ARM::Vec3& position, const ARM::Vec3& direction);
	~AudioSource();

	void UpdateSourcePosVelOri();
	void DefineConeParameters(float outerGain, double innerAngle, double outerAngle);
	
	void SetPitch(float value);
	void SetVolume(float value);
	void Stream();
	void Loop();

private:
	void SubmitBuffer();
	void Play();
	void Stop();
	void Pause();
};
}
}