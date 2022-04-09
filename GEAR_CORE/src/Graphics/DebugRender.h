#pragma once
#include "gear_core_common.h"
#include "Objects/Camera.h"

namespace gear
{
namespace graphics
{
	class GEAR_API DebugRender
	{
		DebugRender() = delete;
		~DebugRender() = delete;

	private:
		static Ref<miru::crossplatform::RenderPass> s_DefaultRenderPass;
		static void* s_Device;
		static miru::crossplatform::Image::Format s_ColourImageFormat;
		static Ref<objects::Camera> s_Camera;

	public:
		static void Initialise(void* device)
		{
			s_Device = device;
		}

		static void Uninitialise()
		{
			s_DefaultRenderPass = nullptr;
			s_Camera = nullptr;
		}

		static const Ref<miru::crossplatform::RenderPass>& GetDefaultRenderPass();
		static Ref<miru::crossplatform::ImageView> CreateColourImageView();
		static Ref<miru::crossplatform::Framebuffer> CreateFramebuffer(const Ref<miru::crossplatform::ImageView>& colourImageView);
		static Ref<objects::Camera>& GetCamera();
	
	};
}
}