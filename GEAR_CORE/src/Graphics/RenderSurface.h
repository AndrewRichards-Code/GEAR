#pragma once

#include "gear_core_common.h"

namespace gear 
{
namespace graphics 
{
	class RenderSurface
	{
	public:
		struct CreateInfo
		{
			std::string										debugName;
			void*											window;
			miru::crossplatform::GraphicsAPI::API			api;
			uint32_t										width;
			uint32_t										height;
			bool											vSync;
			miru::crossplatform::Swapchain::BPC_ColourSpace bpcColourSpace;
			miru::crossplatform::Image::SampleCountBit		samples;
			miru::debug::GraphicsDebugger::DebuggerType		graphicsDebugger;
		};

	private:
		//Context and Swapchain
		Ref<miru::crossplatform::Context> m_Context;
		miru::crossplatform::Context::CreateInfo m_ContextCI;
		Ref<miru::crossplatform::Swapchain> m_Swapchain;
		miru::crossplatform::Swapchain::CreateInfo m_SwapchainCI;

		Ref<miru::crossplatform::Allocator> m_AttachmentAllocator;

		//DepthImage
		Ref<miru::crossplatform::Image> m_DepthImage;
		miru::crossplatform::Image::CreateInfo m_DepthImageCI;
		Ref<miru::crossplatform::ImageView> m_DepthImageView;
		miru::crossplatform::ImageView::CreateInfo m_DepthImageViewCI;
		
		//MSAAColourImage
		Ref<miru::crossplatform::Image> m_MSAAColourImage;
		miru::crossplatform::Image::CreateInfo m_MSAAColourImageCI;
		Ref<miru::crossplatform::ImageView> m_MSAAColourImageView;
		miru::crossplatform::ImageView::CreateInfo m_MSAAColourImageViewCI;
		
		//ColourImage
		Ref<miru::crossplatform::Image> m_ColourImage;
		miru::crossplatform::Image::CreateInfo m_ColourImageCI;
		Ref<miru::crossplatform::ImageView> m_ColourImageView;
		miru::crossplatform::ImageView::CreateInfo m_ColourImageViewCI;

		//MSAAEmissiveImage
		Ref<miru::crossplatform::Image> m_MSAAEmissiveImage;
		miru::crossplatform::Image::CreateInfo m_MSAAEmissiveImageCI;
		Ref<miru::crossplatform::ImageView> m_MSAAEmissiveImageView;
		miru::crossplatform::ImageView::CreateInfo m_MSAAEmissiveImageViewCI;

		//EmissiveImage
		Ref<miru::crossplatform::Image> m_EmissiveImage;
		miru::crossplatform::Image::CreateInfo m_EmissiveImageCI;
		Ref<miru::crossplatform::ImageView> m_EmissiveImageView;
		miru::crossplatform::ImageView::CreateInfo m_EmissiveImageViewCI;

		//MainRenderPass and MainFramebuffer
		Ref<miru::crossplatform::RenderPass> m_MainRenderPass;
		miru::crossplatform::RenderPass::CreateInfo m_MainRenderPassCI;
		Ref<miru::crossplatform::Framebuffer> m_MainFramebuffers[2];
		miru::crossplatform::Framebuffer::CreateInfo m_MainFramebufferCI;

		//HDRRenderPass and HDRFramebuffer
		Ref<miru::crossplatform::RenderPass> m_HDRRenderPass;
		miru::crossplatform::RenderPass::CreateInfo m_HDRRenderPassCI;
		Ref<miru::crossplatform::Framebuffer> m_HDRFramebuffers[2];
		miru::crossplatform::Framebuffer::CreateInfo m_HDRFramebufferCI;

		CreateInfo m_CI;
		int m_CurrentWidth, m_CurrentHeight;
		const miru::crossplatform::Image::Format m_HDRFormat = miru::crossplatform::Image::Format::R16G16B16A16_SFLOAT;

	public:
		RenderSurface(CreateInfo* pCreateInfo);
		~RenderSurface();

		const CreateInfo& GetCreateInfo() { return m_CI; }
		
		void Resize(int width, int height);

		inline const Ref<miru::crossplatform::Context> GetContext() const { return m_Context; };
		inline const Ref<miru::crossplatform::Swapchain>& GetSwapchain() const { return m_Swapchain; };
		inline void* GetDevice() const { return m_Context->GetDevice(); }
		inline const Ref<miru::crossplatform::RenderPass>& GetMainRenderPass() const { return m_MainRenderPass; }
		inline const Ref<miru::crossplatform::RenderPass>& GetHDRRenderPass() const { return m_HDRRenderPass; }
		inline const Ref<miru::crossplatform::Framebuffer>* GetMainFramebuffers() { return m_MainFramebuffers; }
		inline const Ref<miru::crossplatform::Framebuffer>* GetHDRFramebuffers() { return m_HDRFramebuffers; }

		inline const miru::crossplatform::GraphicsAPI::API& GetGraphicsAPI() const { return m_CI.api; }
		inline int GetWidth() const { return m_CurrentWidth; }
		inline int GetHeight() const { return m_CurrentHeight; }
		inline float GetRatio() const { return ((float)m_CurrentWidth / (float)m_CurrentHeight); }
		inline bool& Resized() const { return m_Swapchain->m_Resized; }
		std::string GetGraphicsAPIVersion() const;
		std::string GetDeviceName() const;
		inline std::string GetAntiAliasingValue() const { return std::to_string(static_cast<uint32_t>(m_CI.samples)); }
	
	private:
		void CreateAttachments();
		
		void CreateMainRenderPass();
		void CreateHDRRenderPass();

		void CreateMainFramebuffer();
		void CreateHDRFramebuffer();
		
		friend class Window;
	};
}
}