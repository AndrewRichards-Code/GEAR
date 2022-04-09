#pragma once
#include "ObjectInterface.h"
#include "Camera.h"
#include "Probe.h"

#define GEAR_MAX_LIGHTS 8

namespace gear 
{
namespace objects 
{
	class GEAR_API Light : public ObjectInterface
	{
	public:
		enum class Type : uint32_t
		{
			POINT = 0,
			DIRECTIONAL = 1,
			SPOT = 2,
		};

		struct CreateInfo : public ObjectInterface::CreateInfo
		{
			Type			type;
			mars::float4	colour;
			float			spotInnerAngle; //Radians
			float			spotOuterAngle; //Radians
		};

	private:
		typedef graphics::UniformBufferStructures::Lights LightUB;
		static Ref<graphics::Uniformbuffer<LightUB>> s_UB;

		static int s_NumOfLights;
		size_t m_LightID;
		Ref<Probe> m_Probe;

	public:
		CreateInfo m_CI;

	public:
		Light(CreateInfo* pCreateInfo);
		~Light();

		const Ref<graphics::Uniformbuffer<LightUB>>& GetUB() const { return s_UB; };
		const Ref<Probe>& GetProbe() const { return m_Probe; };

		//Update the light from the current state of Light::CreateInfo m_CI.
		void Update(const Transform& transform);

	protected:
		bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) override;

	private:
		void InitialiseUB();
		void CreateProbe();
	};
}
}