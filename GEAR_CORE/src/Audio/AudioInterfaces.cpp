#include "gear_core_common.h"
#include "Audio/AudioInterfaces.h"

#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
#define XAUDIO2_HELPER_FUNCTIONS
#include <xaudio2.h>
#include <x3daudio.h>
#endif

using namespace gear;
using namespace audio;

//////////////////////////////////////////////
//----------AudioListenerInterface----------//
//////////////////////////////////////////////

AudioListenerInterface::API AudioListenerInterface::s_API = AudioListenerInterface::API::UNKNOWN;

AudioListenerInterface::AudioListenerInterface(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if(s_API == API::UNKNOWN)
	{
		#if !defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
		s_API = API::OPEN_AL;
		#else
		s_API = m_CI.audioAPI;
		#endif
	}

	switch (s_API)
	{
	default:
	case AudioListenerInterface::API::UNKNOWN:
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::NOT_SUPPORTED, "Unknown AudioAPI.");
		break;
	}
	case AudioListenerInterface::API::XAUDIO2:
	{
		XAudio2_CreateXAudio2AndMasteringVoice();
		break;
	}
	}
}
AudioListenerInterface::~AudioListenerInterface()
{
	switch (s_API)
	{
	default:
	case AudioListenerInterface::API::UNKNOWN:
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::NOT_SUPPORTED, "Unknown AudioAPI.");
		break;
	}
	case AudioListenerInterface::API::XAUDIO2:
	{
		XAudio2_DestroyXAudio2AndMasteringVoice();
		break;
	}
	}
}

//----------XAUDIO2----------
#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>

void AudioListenerInterface::XAudio2_CreateXAudio2AndMasteringVoice()
{
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to CoInitialise XAudio2.");
	}

	UINT32 flags = 0;
	if (FAILED(XAudio2Create(&m_IXAudio2, flags, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Create XAudio2.");
	}

	#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) && defined(_DEBUG)
	XAUDIO2_DEBUG_CONFIGURATION debugConfig = { XAUDIO2_LOG_WARNINGS, XAUDIO2_LOG_WARNINGS, 1, 1, 1, 1 };
	m_IXAudio2->SetDebugConfiguration(&debugConfig, NULL);
	//	  To see the trace output, you need to view ETW logs for this application:
	//    Go to Control Panel, Administrative Tools, Event Viewer.
	//    View->Show Analytic and Debug Logs.
	//    Applications and Services Logs / Microsoft / Windows / XAudio2. 
	//    Right click on Microsoft Windows XAudio2 debug logging, Properties, then Enable Logging, and hit OK 
	#endif

	std::wstring deviceID;
	IMMDeviceEnumerator* deviceEnumerator;
	if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator)))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Create Audio Device Enumerator.");
	}

	auto GetDefaultAudioEndpointDeviceID = [&]()
	{
		IMMDevice* device;
		if (FAILED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device)))
		{
			GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Get Default Audio Endpoint.");
		}

		LPWSTR wDeviceID_c_str;
		if (FAILED(device->GetId(&wDeviceID_c_str)))
		{
			GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Get DeviceID for Default Audio Endpoint.");
		}
		deviceID = wDeviceID_c_str;

		CoTaskMemFree((LPVOID)wDeviceID_c_str);
		device->Release();
	};
	auto GetAudioEndpointDeviceID = [&](const std::wstring& _friendlyName)
	{
		IMMDeviceCollection* deviceCollections;
		if (FAILED(deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &deviceCollections)))
		{
			GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Enumerate all Audio Endpoints.");
		}
		UINT deviceCollectionCount;
		if (FAILED(deviceCollections->GetCount(&deviceCollectionCount)))
		{
			GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Get Count for all Audio Endpoints.");
		}
		for (UINT i = 0; i < deviceCollectionCount; i++)
		{
			IMMDevice* device;
			if (FAILED(deviceCollections->Item(i, &device)))
			{
				GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Get Device for Audio Endpoint: %u.", i);
			}

			LPWSTR wDeviceID_c_str;
			if (FAILED(device->GetId(&wDeviceID_c_str)))
			{
				GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Get DeviceID for Audio Endpoint: %u.", i);
			}

			IPropertyStore* propertyStore;
			if (FAILED(device->OpenPropertyStore(STGM_READ, &propertyStore)))
			{
				GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Open Property Store for Audio Endpoint: %u.", i);
			}

			PROPVARIANT friendlyName;
			PropVariantInit(&friendlyName);
			if (FAILED(propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName)))
			{
				GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Get Value from Property Store for Audio Endpoint: %u.", i);
			}

			if (friendlyName.pwszVal)
			{
				std::wstring name(friendlyName.pwszVal);
				if (name.find(_friendlyName) != std::wstring::npos)
				{
					deviceID = wDeviceID_c_str;
				}
			}

			PropVariantClear(&friendlyName);
			propertyStore->Release();
			CoTaskMemFree((LPVOID)wDeviceID_c_str);
			device->Release();
		}
		deviceCollections->Release();
	};

	switch (m_CI.endPointDevice)
	{
		default:
		case EndPointDevice::DEFAULT:
		{
			GetDefaultAudioEndpointDeviceID();
			break;
		}
		case EndPointDevice::HEADPHONES_XBOX_CONTROLLER:
		{
			GetAudioEndpointDeviceID(L"Headphones (Xbox Controller)");
			break;
		}
	}
	deviceEnumerator->Release();

	if (FAILED(m_IXAudio2->CreateMasteringVoice(&m_IXAudio2MasteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, (deviceID.empty() ? nullptr : deviceID.c_str()), nullptr, AudioCategory_GameEffects)))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Create XAudio2MasteringVoice.");
	}
}

void AudioListenerInterface::XAudio2_DestroyXAudio2AndMasteringVoice()
{
	m_IXAudio2MasteringVoice->DestroyVoice();
	if (!m_IXAudio2->Release())
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::INIT_FAILED, "Failed to Release XAudio2.");
	}
	CoUninitialize();
}
#else
void AudioListenerInterface::XAudio2_CreateXAudio2AndMasteringVoice() {}
void AudioListenerInterface::XAudio2_DestroyXAudio2AndMasteringVoice() {}
#endif

////////////////////////////////////////////
//----------AudioSourceInterface----------//
////////////////////////////////////////////

AudioSourceInterface::AudioSourceInterface(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	m_API = m_CI.pAudioListener->GetAPI();
	m_WavData = utils::stream_wav(m_CI.filepath);

	switch (m_API)
	{
	default:
	case AudioListenerInterface::API::UNKNOWN:
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::NOT_SUPPORTED, "Unknown AudioAPI.");
		break;
	}
	case AudioListenerInterface::API::XAUDIO2:
	{
		CreateIXAudio2SourceVoiceAndX3DAUDIO();
		break;
	}
	}
}

AudioSourceInterface::~AudioSourceInterface()
{
	switch (m_API)
	{
	default:
	case AudioListenerInterface::API::UNKNOWN:
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::NOT_SUPPORTED, "Unknown AudioAPI.");
		break;
	}
	case AudioListenerInterface::API::XAUDIO2:
	{
		DestroyIXAudio2SourceVoiceAndX3DAUDIO();
		break;
	}
	}
}

void AudioSourceInterface::Stream()
{
	switch (m_API)
	{
	default:
	case AudioListenerInterface::API::UNKNOWN:
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::NOT_SUPPORTED, "Unknown AudioAPI.");
		break;
	}
	case AudioListenerInterface::API::XAUDIO2:
	{
		Stream_Internal();
		break;
	}
	}
}

void AudioSourceInterface::SetPitch(float value)
{
	if (value > 12.0f || value < -12.0f)
	{
		GEAR_WARN(ErrorCode::AUDIO | ErrorCode::INVALID_VALUE, "Input value out of range. Pitch has not been changed.");
		return;
	}

	switch (m_API)
	{
	default:
	case AudioListenerInterface::API::UNKNOWN:
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::NOT_SUPPORTED, "Unknown AudioAPI.");
		break;
	}
	case AudioListenerInterface::API::XAUDIO2:
	{
		SetPitch_Internal(value);
		break;
	}
	}
}

void AudioSourceInterface::SetVolume(float value)
{
	//value is in dB, linear should be from 0.0 to +inf 
	float linear = powf(10.0f, (value / 20.0f));

	if (linear < 0.0f)
	{
		GEAR_WARN(ErrorCode::AUDIO | ErrorCode::INVALID_VALUE, "Calculated value out of range. Volume has not been changed.");
		return;
	}

	switch (m_API)
	{
	default:
	case AudioListenerInterface::API::UNKNOWN:
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::NOT_SUPPORTED, "Unknown AudioAPI.");
		break;
	}
	case AudioListenerInterface::API::XAUDIO2:
	{
		SetVolume_Internal(linear);
		break;
	}
	}
}

//----------XAUDIO2----------
#if defined(GEAR_PLATFORM_WINDOWS_OR_XBOX)
void AudioSourceInterface::CreateIXAudio2SourceVoiceAndX3DAUDIO()
{
	WAVEFORMATEX wfx;
	wfx.wFormatTag = static_cast<WORD>(m_WavData->formatTag);
	wfx.nChannels = static_cast<WORD>(m_WavData->channels);
	wfx.nSamplesPerSec = static_cast<DWORD>(m_WavData->sampleRate);
	wfx.nAvgBytesPerSec = static_cast<DWORD>(m_WavData->sampleRate * m_WavData->blockAlign);
	wfx.nBlockAlign = static_cast<WORD>(m_WavData->blockAlign);
	wfx.wBitsPerSample = static_cast<WORD>(m_WavData->bitsPerSample);
	wfx.cbSize = 0;

	IXAudio2*& xAudio2 = m_CI.pAudioListener->m_IXAudio2;
	if (FAILED(xAudio2->CreateSourceVoice(&m_IXAudio2SourceVoice, &wfx)))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::FUNC_FAILED, "Failed to Create IXAudio2SourceVoice.");
	}

	SubmitBuffer();
	SubmitBuffer();

	/*
	IXAudio2MasteringVoice*& xAudio2MasteringVoice = m_CI.pAudioListener->m_IXAudio2MasteringVoice;
	DWORD channelMask;
	xAudio2MasteringVoice->GetChannelMask(&channelMask);
	X3DAudioInitialize(static_cast<UINT32>(channelMask), X3DAUDIO_SPEED_OF_SOUND, m_X3DAudioHandle);

	auto ToX3DAUDIO_VECTOR = [](const mars::float3& other) -> X3DAUDIO_VECTOR
	{
		return X3DAUDIO_VECTOR(other.x, other.y, other.z);
	};

	m_X3DAudioListener.OrientFront = ToX3DAUDIO_VECTOR(mars::float3(m_CI.pAudioListener->m_Transform.orientation.ToRotationMatrix4<float>() * mars::float4(0.0f, 0.0f, -1.0f, 0.0f)));
	m_X3DAudioListener.OrientTop = ToX3DAUDIO_VECTOR(mars::float3(m_CI.pAudioListener->m_Transform.orientation.ToRotationMatrix4<float>() * mars::float4(0.0f, +1.0f, 0.0f, 0.0f)));
	m_X3DAudioListener.Position = ToX3DAUDIO_VECTOR(m_CI.pAudioListener->m_Transform.translation);
	m_X3DAudioListener.Velocity = { 0.0f, 0.0f, 0.0f };
	m_X3DAudioListener.pCone = nullptr;

	m_X3DAudioEmitter.pCone = nullptr;									
	m_X3DAudioEmitter.OrientFront = ToX3DAUDIO_VECTOR(mars::float3(m_Transform.orientation.ToRotationMatrix4<float>() * mars::float4(0.0f, 0.0f, -1.0f, 0.0f)));
	m_X3DAudioEmitter.OrientTop = ToX3DAUDIO_VECTOR(mars::float3(m_Transform.orientation.ToRotationMatrix4<float>() * mars::float4(0.0f, +1.0f, 0.0f, 0.0f)));
	m_X3DAudioEmitter.Position = ToX3DAUDIO_VECTOR(m_Transform.translation);
	m_X3DAudioEmitter.Velocity = { 0.0f, 0.0f, 0.0f };
	m_X3DAudioEmitter.InnerRadius;						
	m_X3DAudioEmitter.InnerRadiusAngle;					
	m_X3DAudioEmitter.ChannelCount;						
	m_X3DAudioEmitter.ChannelRadius;						
	m_X3DAudioEmitter.pChannelAzimuths;					
	m_X3DAudioEmitter.pVolumeCurve;		
	m_X3DAudioEmitter.pLFECurve;			
	m_X3DAudioEmitter.pLPFDirectCurve;	
	m_X3DAudioEmitter.pLPFReverbCurve;	
	m_X3DAudioEmitter.pReverbCurve;		
	m_X3DAudioEmitter.CurveDistanceScaler;				
	m_X3DAudioEmitter.DopplerScaler;						

	xAudio2MasteringVoice->GetVoiceDetails(&m_XAudio2VoiceDetails);

	m_X3DAudioDSPSettings.pMatrixCoefficients;			
	m_X3DAudioDSPSettings.pDelayTimes;					
	m_X3DAudioDSPSettings.SrcChannelCount;				
	m_X3DAudioDSPSettings.DstChannelCount;				
	m_X3DAudioDSPSettings.LPFDirectCoefficient;			
	m_X3DAudioDSPSettings.LPFReverbCoefficient;			
	m_X3DAudioDSPSettings.ReverbLevel;					
	m_X3DAudioDSPSettings.DopplerFactor;				
	m_X3DAudioDSPSettings.EmitterToListenerAngle;		
	m_X3DAudioDSPSettings.EmitterToListenerDistance;	
	m_X3DAudioDSPSettings.EmitterVelocityComponent;		
	m_X3DAudioDSPSettings.ListenerVelocityComponent;
	*/
}

void AudioSourceInterface::DestroyIXAudio2SourceVoiceAndX3DAUDIO()
{
	m_IXAudio2SourceVoice->DestroyVoice();
}

void AudioSourceInterface::SubmitBuffer()
{
	utils::get_next_wav_block(m_WavData);

	XAUDIO2_BUFFER buffer;
	buffer.Flags = 0;
	switch (m_WavData->nextBuffer)
	{
	case 1:
	{
		buffer.AudioBytes = static_cast<UINT32>(m_WavData->buffer2.size());
		buffer.pAudioData = reinterpret_cast<BYTE*>(m_WavData->buffer2.data());
		break;
	}
	case 2:
	{
		buffer.AudioBytes = static_cast<UINT32>(m_WavData->buffer1.size());
		buffer.pAudioData = reinterpret_cast<BYTE*>(m_WavData->buffer1.data());
		break;
	}
	case 0:
	{
		m_Ended = true;
		Stop();
		return;
	}
	}
	buffer.PlayBegin = 0;
	buffer.PlayLength = 0;
	buffer.LoopBegin = 0;
	buffer.LoopLength = 0;
	buffer.LoopCount = 0;
	buffer.pContext = nullptr;

	if (FAILED(m_IXAudio2SourceVoice->SubmitSourceBuffer(&buffer)))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::FUNC_FAILED, "Failed to Submit XAUDIO2_BUFFER to IXAudio2SourceVoice.");
	}
}
void AudioSourceInterface::Play()
{
	if (FAILED(m_IXAudio2SourceVoice->Start()))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::FUNC_FAILED, "Failed to Start IXAudio2SourceVoice.");
	}
}
void AudioSourceInterface::Stop()
{
	if (FAILED(m_IXAudio2SourceVoice->Stop()))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::FUNC_FAILED, "Failed to Stop IXAudio2SourceVoice.");
	}
	if (FAILED(m_IXAudio2SourceVoice->FlushSourceBuffers()))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::FUNC_FAILED, "Failed to Flush buffers for IXAudio2SourceVoice.");
	}
	SubmitBuffer();
	SubmitBuffer();
}
void AudioSourceInterface::Pause()
{
	if (FAILED(m_IXAudio2SourceVoice->Stop()))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::FUNC_FAILED, "Failed to Stop IXAudio2SourceVoice.");
	}
}
void AudioSourceInterface::Stream_Internal()
{
	if (!m_Ended)
	{
		Play();
	}

	XAUDIO2_VOICE_STATE state;
	m_IXAudio2SourceVoice->GetState(&state);

	if (state.BuffersQueued < 2)
	{
		SubmitBuffer();
	}
}
void AudioSourceInterface::SetPitch_Internal(float value)
{
	float freqRatio;
	m_IXAudio2SourceVoice->GetFrequencyRatio(&freqRatio);
	float currentSemitone = XAudio2FrequencyRatioToSemitones(freqRatio);
	float newFreqRatio = XAudio2SemitonesToFrequencyRatio(currentSemitone + value);

	if (FAILED(m_IXAudio2SourceVoice->SetFrequencyRatio(newFreqRatio)))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::FUNC_FAILED, "Failed to SetFrequencyRatio IXAudio2SourceVoice.");
	}
}
void AudioSourceInterface::SetVolume_Internal(float value)
{
	if (FAILED(m_IXAudio2SourceVoice->SetVolume(value)))
	{
		GEAR_ASSERT(ErrorCode::AUDIO | ErrorCode::FUNC_FAILED, "Failed to SetVolume IXAudio2SourceVoice.");
	}
}
#else
void AudioSourceInterface::CreateIXAudio2SourceVoiceAndX3DAUDIO() {}
void AudioSourceInterface::DestroyIXAudio2SourceVoiceAndX3DAUDIO() {}
void AudioSourceInterface::SubmitBuffer() {}
void AudioSourceInterface::Play() {}
void AudioSourceInterface::Stop() {}
void AudioSourceInterface::Pause() {}
void AudioSourceInterface::Stream_Internal() {}
void AudioSourceInterface::SetPitch_Internal(float value) {}
void AudioSourceInterface::SetVolume_Internal(float value) {}
#endif