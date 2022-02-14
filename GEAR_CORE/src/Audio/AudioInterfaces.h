#pragma once
#include "gear_core_common.h"
#include "Utils/FileUtils.h"
#include "Objects/Transform.h"

#undef OPENAL

namespace gear
{
namespace audio
{
	class GEAR_API AudioListenerInterface
	{
	public:
		enum class API
		{
			UNKNOWN,
			OPENAL,
			XAUDIO2
		};
		enum class EndPointDevice : uint32_t
		{
			DEFAULT,
			HEADPHONES_XBOX_CONTROLLER,
			//HEADSET_MICROPHONE_XBOX_CONTROLLER
		};

		struct CreateInfo
		{
			API				audioAPI;
			EndPointDevice	endPointDevice;
		};

	private:
		CreateInfo m_CI;
		static API s_API;
	
	public:
		objects::Transform m_Transform;

	public:
		AudioListenerInterface(CreateInfo* pCreateInfo);
		~AudioListenerInterface();

		const CreateInfo& GetCreateInfo() { return m_CI; }

		static inline const API& GetAPI() { return s_API; }

	//OpenAL
	public:
		ALCdevice* m_ALCdevice;
		ALCcontext* m_ALCcontext;

	private:
		void OpenAL_CreateDeviceAndContext();
		void OpenAL_DestroyDeviceAndContext();

	//XAudio2
	#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
	public:
		IXAudio2* m_IXAudio2;
		IXAudio2MasteringVoice* m_IXAudio2MasteringVoice;

	private:
		void XAudio2_CreateXAudio2AndMasteringVoice();
		void XAudio2_DestroyXAudio2AndMasteringVoice();
	#endif

	};

	class AudioSourceInterface
	{
	public:
		struct CreateInfo
		{
			std::string					filepath;
			Ref<AudioListenerInterface>	pAudioListener;
		};

	private:
		CreateInfo m_CI;
		Ref<file_utils::WavData>m_WavData;
		bool m_Ended = false;

		AudioListenerInterface::API m_API;

	public:
		objects::Transform m_Transform;

	public:
		AudioSourceInterface(CreateInfo* pCreateInfo);
		~AudioSourceInterface();

		const CreateInfo& GetCreateInfo() { return m_CI; }

		void Stream();
		
		//In semitones (-12.0f < value < 12.0f).
		void SetPitch(float value);  
		//In decibels.
		void SetVolume(float value);  

		inline void Loop() { m_WavData->loopBufferQueue = true; };
		inline void Unloop() { m_WavData->loopBufferQueue = false; };

		inline const AudioListenerInterface::API& GetAPI() const { return m_API; }
		inline const Ref<file_utils::WavData>& GetWavData() const { return m_WavData; }

		//OpenAL
	private:
		ALuint m_BufferID[2];
		ALuint m_SourceID;

	private:
		void OpenAL_CreateBufferAndSource();
		void OpenAL_DestroyBufferAndSource();
		
		void OpenAL_SubmitBuffer();
		void OpenAL_Play();
		void OpenAL_Stop();
		void OpenAL_Pause();
		void OpenAL_Stream();
		
		void OpenAL_SetPitch(float value);
		void OpenAL_SetVolume(float value);

		//XAudio2
#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
	private:
		IXAudio2SourceVoice* m_IXAudio2SourceVoice;

		X3DAUDIO_HANDLE m_X3DAudioHandle;
		X3DAUDIO_LISTENER m_X3DAudioListener;
		X3DAUDIO_EMITTER m_X3DAudioEmitter;
		X3DAUDIO_DSP_SETTINGS m_X3DAudioDSPSettings;
		XAUDIO2_VOICE_DETAILS m_XAudio2VoiceDetails;

	private:
		void XAudio2_CreateIXAudio2SourceVoiceAndX3DAUDIO();
		void XAudio2_DestroyIXAudio2SourceVoiceAndX3DAUDIO();
		
		void XAudio2_SubmitBuffer();
		void XAudio2_Play();
		void XAudio2_Stop();
		void XAudio2_Pause();
		void XAudio2_Stream();

		void XAudio2_SetPitch(float value);
		void XAudio2_SetVolume(float value);
#endif
	};

	#define GEAR_AUDIO_MUTE_VOLUME -3.402823466e+38f
}
}