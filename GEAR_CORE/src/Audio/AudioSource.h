#pragma once

#include "gear_core_common.h"
#include "AudioListener.h"

namespace gear 
{
namespace audio 
{
	class AudioSource
	{
	public:
		struct CreateInfo
		{
			std::string					filepath;
			gear::Ref<AudioListener>	pAudioListener;
		};

	private:
		CreateInfo m_CI;

		gear::Ref<AudioSourceInterface> m_ASI;
		AudioSourceInterface::CreateInfo m_ASICI;
	
		std::thread m_AudioStreamThread;
		
		mars::Vec3 m_Position;
		mars::Vec3 m_Velocity = { 0.0f, 0.0f, 0.0f };
		mars::Vec3 m_Direction;

		bool m_Looped;

	public:
		AudioSource(CreateInfo* pCreateInfo);
		~AudioSource();

		const CreateInfo& GetCreateInfo() { return m_CI; }

		//void UpdateSourcePosVelOri();
		//void DefineConeParameters(float outerGain, double innerAngle, double outerAngle);

		void SetPitch(float value);  //In semitones (-12.0f < value < 12.0f).
		void SetVolume(float value);  //In decibels.
		void Stream();
		void Loop();
	};
}
}