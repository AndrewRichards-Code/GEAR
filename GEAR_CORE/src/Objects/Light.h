#pragma once
#include "Objects/ObjectInterface.h"
#include "Graphics/Uniformbuffer.h"

namespace gear 
{
	namespace objects
	{
		class Probe;

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

			static const uint32_t s_MaxLights = 8;

		private:
			typedef graphics::UniformBufferStructures::Lights LightUB;
			static Ref<graphics::Uniformbuffer<LightUB>> s_UB;

			static uint32_t s_NumOfLights;
			size_t m_LightID;
			Ref<Probe> m_Probe;

		public:
			CreateInfo m_CI;

		public:
			Light(CreateInfo* pCreateInfo);
			~Light();

			const size_t GetLightID() { return m_LightID; }
			const Ref<graphics::Uniformbuffer<LightUB>>& GetUB() const { return s_UB; };
			const Ref<Probe>& GetProbe() const { return m_Probe; };

			//Update the light from the current state of Light::CreateInfo m_CI.
			void Update(const Transform& transform) override;

		protected:
			bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) override;

		private:
			void InitialiseUB();
			void CreateProbe();
		};
	}
}