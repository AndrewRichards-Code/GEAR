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

			struct CreateInfo : public ObjectInterface::CreateInfo
			{
				DirectionType			directionType;
				CaptureType				captureType;
				uint32_t				imageWidth;
				uint32_t				imageHeight;				//Only for DirectionType::MONO
				Camera::ProjectionType	projectionType;				//DirectionType::ONMI will force Camera::ProjectionType::PERSPECTIVE
				double					perspectiveHorizonalFOV;	//In radians
				float					orthographicScale;
				float					zNear;
				float					zFar;
			};

			Ref<graphics::Texture> m_ColourTexture;
			graphics::Texture::CreateInfo m_ColourTextureCI;

			Ref<graphics::Texture> m_DepthTexture;
			graphics::Texture::CreateInfo m_DepthTextureCI;

			Ref<graphics::RenderPipeline> m_ShadowRenderPipeline;
			graphics::RenderPipeline::LoadInfo m_ShadowRenderPipelineLI;

			bool m_RenderDebugView = false;
			Ref<graphics::Texture> m_DebugTexture = nullptr;

		private:
			typedef graphics::UniformBufferStructures::ProbeInfo ProbeInfoUB;
			Ref<graphics::Uniformbuffer<ProbeInfoUB>> m_UB;

		public:
			CreateInfo m_CI;

		public:
			Probe(CreateInfo* pCreateInfo);
			~Probe();

			void Update(const Transform& transform) override;
			bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) override;

			const Ref<graphics::Uniformbuffer<ProbeInfoUB>>& GetUB() const { return m_UB; };

		private:
			void CreateTexture(Ref<graphics::Texture>& texture, graphics::Texture::CreateInfo& textureCI, bool colour = true);
			void CreateRenderPipeline();
			void InitialiseUB();
		};
	}
}