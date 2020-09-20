#pragma once

#include "Camera.h"

#define GEAR_MAX_LIGHTS 8

namespace gear 
{
namespace objects 
{

//TODO: Add Shadow mapping.
	class Light
	{
	public:
		enum class LightType : uint32_t
		{
			POINT = 0,		//pos
			DIRECTIONAL = 1,//dir
			SPOT = 2,		//cone, umbra, penumbra
			AREA = 3		//err...
		};

		struct CreateInfo
		{
			std::string debugName;
			void*		device;
			LightType	type;
			mars::Vec4	colour;
			Transform	transform;
		};

	private:
		struct LightUB
		{
			mars::Vec4 colour;
			mars::Vec4 position;
			mars::Vec4 direction;
		};
		typedef std::array<LightUB, GEAR_MAX_LIGHTS> LightUBType;
		gear::Ref<graphics::Uniformbuffer<LightUBType>> m_UB;

		static int s_NumOfLights;
		size_t m_LightID;
		LightType m_Type;

	public:
		CreateInfo m_CI;

	public:
		Light(CreateInfo* pCreateInfo);
		~Light();

		//Update the light from the current state of Light::CreateInfo m_CI.
		void Update();
		gear::Ref<graphics::Uniformbuffer<LightUBType>> GetUB() { return m_UB; };

	private:
		void InitialiseUB();
	};
}
}