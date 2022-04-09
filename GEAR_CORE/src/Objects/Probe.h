#pragma once
#include "ObjectInterface.h"
#include "Graphics/Texture.h"
#include "Graphics/RenderPipeline.h"
#include "Camera.h"

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
			float					zNear;
			float					zFar;
		};

		Ref<graphics::Texture> m_ColourTexture;
		graphics::Texture::CreateInfo m_ColourTextureCI;
		
		Ref<graphics::Texture> m_DepthTexture;
		graphics::Texture::CreateInfo m_DepthTextureCI;

		Ref<miru::crossplatform::RenderPass> m_RenderPass;
		miru::crossplatform::RenderPass::CreateInfo m_RenderPassCI;

		Ref<miru::crossplatform::Framebuffer> m_Framebuffer;
		miru::crossplatform::Framebuffer::CreateInfo m_FramebufferCI;

		Ref<graphics::RenderPipeline> m_ShadowRenderPipeline;
		graphics::RenderPipeline::LoadInfo m_ShadowRenderPipelineLI;

		Ref<miru::crossplatform::Framebuffer> m_DebugFramebuffer[2] = { 0, 0 };

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
		void CreateRenderPass();
		void CreateFramebuffer();
		void CreateRenderPipeline();
		void InitialiseUB();
	};
}
}