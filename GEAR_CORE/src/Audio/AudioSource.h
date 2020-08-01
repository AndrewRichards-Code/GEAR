#pragma once

#include "gear_core_common.h"
#include "Utils/FileUtils.h"

namespace gear {
namespace audio {
class AudioSource
{
private:
	const char* m_FilePath;
	unsigned int m_BufferID[2];
	unsigned int m_SourceID;

	std::shared_ptr<file_utils::WavData> m_WavData;
	unsigned int m_Format;

	mars::Vec3 m_Position;
	mars::Vec3 m_Velocity = { 0.0f, 0.0f, 0.0f };
	mars::Vec3 m_Direction;

	bool m_Looped = false;
	bool m_Ended = false;
	
public:
	AudioSource(const char* filepath, const mars::Vec3& position, const mars::Vec3& direction);
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