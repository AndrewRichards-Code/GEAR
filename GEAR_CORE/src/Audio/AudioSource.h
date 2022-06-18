#pragma once
#include "gear_core_common.h"
#include "Audio/AudioListener.h"

namespace gear
{
	namespace audio
	{
		class GEAR_API AudioSource
		{
		public:
			struct CreateInfo
			{
				std::string			filepath;
				Ref<AudioListener>	pAudioListener;
			};

		private:
			CreateInfo m_CI;

			Ref<AudioSourceInterface> m_ASI;
			AudioSourceInterface::CreateInfo m_ASICI;

			std::thread m_AudioStreamThread;

			mars::float3 m_Position;
			mars::float3 m_Velocity = { 0.0f, 0.0f, 0.0f };
			mars::float3 m_Direction;

			bool m_Looped;

		public:
			AudioSource(CreateInfo* pCreateInfo);
			~AudioSource();

			const CreateInfo& GetCreateInfo() { return m_CI; }

			//void UpdateSourcePosVelOri();
			//void DefineConeParameters(float outerGain, double innerAngle, double outerAngle);

			//In semitones (-12.0f < value < 12.0f).
			void SetPitch(float value);
			//In decibels.
			void SetVolume(float value);

			void Stream();
			void Loop();
		};
	}
}