#pragma once
#include "ObjectInterface.h"
#include "Camera.h"

#define GEAR_MAX_LIGHTS 8

namespace gear 
{
namespace objects 
{
	//TODO: Add Shadow mapping.
	class GEAR_API Light : public ObjectInterface
	{
	public:
		enum class LightType : uint32_t
		{
			POINT = 0,		//pos
			DIRECTIONAL = 1,//dir
			SPOT = 2,		//cone, umbra, penumbra
			AREA = 3		//err...
		};

		struct CreateInfo : public ObjectInterface::CreateInfo
		{
			LightType		type;
			mars::float4	colour;
		};

	private:
		typedef graphics::UniformBufferStructures::Lights LightUB;
		static Ref<graphics::Uniformbuffer<LightUB>> s_UB;

		static int s_NumOfLights;
		size_t m_LightID;
		LightType m_Type;

	public:
		CreateInfo m_CI;

	public:
		Light(CreateInfo* pCreateInfo);
		~Light();

		const Ref<graphics::Uniformbuffer<LightUB>>& GetUB() const { return s_UB; };

		//Update the light from the current state of Light::CreateInfo m_CI.
		void Update(const Transform& transform);

	protected:
		bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) override;

	private:
		void InitialiseUB();
	};
}
}