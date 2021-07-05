#pragma once

#include "gear_core_common.h"
#include "AudioInterfaces.h"
#include "Objects/Transform.h"

namespace gear 
{
namespace audio 
{
	class AudioListener
	{
	public:
		struct CreateInfo
		{
			AudioListenerInterface::API				audioAPI;
			AudioListenerInterface::EndPointDevice	endPointDevice;
		};

	private:
		CreateInfo m_CI;
		
		Ref<AudioListenerInterface> m_ALI;
		AudioListenerInterface::CreateInfo m_ALICI;

		float m_ListenerPosition[3];
		float m_ListenerVelocity[3];
		float m_ListenerOrientation[6];

	public:
		AudioListener(CreateInfo* pCreateInfo);
		~AudioListener();

		const CreateInfo& GetCreateInfo() { return m_CI; }
		const Ref<AudioListenerInterface>& GetAudioListenerInterface() { return m_ALI; }

		//void UpdateListener(const objects::Transform& transform);
	};
}
}

