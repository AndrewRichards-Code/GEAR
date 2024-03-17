#pragma once
#include "ObjectInterface.h"
#include "Graphics/Texture.h"
#include "Graphics/RenderPipeline.h"
#include "Objects/Camera.h"

namespace gear
{
	namespace objects
	{
		class GEAR_API Probe : public ObjectInterface, public ObjectViewInterface
		{
		public:
			enum class DirectionType : uint32_t
			{
				MONO,
				OMNI
			};
			enum class CaptureType : uint32_t
			{
				SHADOW,
				REFLECTION
			};

			static constexpr uint32_t MaxShadowCascades = _countof(graphics::UniformBufferStructures::ProbeInfo::proj);

			struct CreateInfo : public ObjectInterface::CreateInfo
			{
				DirectionType			directionType;
				CaptureType				captureType;
				uint32_t				imageSize;
				Camera::ProjectionType	projectionType;								//DirectionType::ONMI will force Camera::ProjectionType::PERSPECTIVE
				double					perspectiveHorizonalFOV;					//In radians
				float					zNear;										//Only for Camera::ProjectionType::PERSPECTIVE
				float					zFar;										//Only for Camera::ProjectionType::PERSPECTIVE
				uint32_t				shadowCascades = 1;
				float					shadowCascadeDistances[MaxShadowCascades];	//Far planes for the cascades
				Ref<Camera>				viewCamera;
			};

			Ref<graphics::Texture> m_ColourTexture;
			Ref<graphics::Texture> m_DepthTexture;

			bool m_ViewCameraUpdated = false;

			bool m_RenderDebugView = false;
			Ref<graphics::Texture> m_DebugTexture = nullptr;

			miru::base::ImageViewRef m_DebugImageViews[MaxShadowCascades] = {nullptr, nullptr, nullptr, nullptr};

		private:
			typedef graphics::UniformBufferStructures::ProbeInfo ProbeInfoUB;
			Ref<graphics::Uniformbuffer<ProbeInfoUB>> m_UB;

			DirectionType m_CurrentDirectionType = DirectionType::MONO;
			CaptureType m_CurrentCaptureType = CaptureType::SHADOW;
			uint32_t m_CurrentImageSize = 0;

		public:
			CreateInfo m_CI;

		public:
			Probe(CreateInfo* pCreateInfo);
			~Probe();

			void Update(const Transform& transform) override;
			const Ref<graphics::Uniformbuffer<ProbeInfoUB>>& GetUB() const { return m_UB; }

			void UpdateFromViewCamera();

		protected:
			bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) override;

		private:
			void InitialiseTextures();
			void CreateTexture(Ref<graphics::Texture>& texture, bool colour = true);
			void InitialiseUB();
		};
	}
}