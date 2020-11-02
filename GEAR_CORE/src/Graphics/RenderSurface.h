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
			std::string									debugName;
			void*										window;
			miru::crossplatform::GraphicsAPI::API		api;
			uint32_t									width;
			uint32_t									height;
			bool										vSync;
			miru::crossplatform::Image::SampleCountBit	samples;
			miru::debug::GraphicsDebugger::DebuggerType graphicsDebugger;
		};

	private:
		//Context and Swapchain
		miru::Ref<miru::crossplatform::Context> m_Context;
		miru::crossplatform::Context::CreateInfo m_ContextCI;
		miru::Ref<miru::crossplatform::Swapchain> m_Swapchain;
		miru::crossplatform::Swapchain::CreateInfo m_SwapchainCI;

		//ColourImage
		miru::Ref<miru::crossplatform::Allocator> m_AttachmentAllocator;
		miru::Ref<miru::crossplatform::Image> m_ColourImage;
		miru::crossplatform::Image::CreateInfo m_ColourImageCI;
		miru::Ref<miru::crossplatform::ImageView> m_ColourImageView;
		miru::crossplatform::ImageView::CreateInfo m_ColourImageViewCI;

		//DepthImage
		miru::Ref<miru::crossplatform::Image> m_DepthImage;
		miru::crossplatform::Image::CreateInfo m_DepthImageCI;
		miru::Ref<miru::crossplatform::ImageView> m_DepthImageView;
		miru::crossplatform::ImageView::CreateInfo m_DepthImageViewCI;

		//RenderPass and Framebuffer
		miru::Ref<miru::crossplatform::RenderPass> m_RenderPass;
		miru::crossplatform::RenderPass::CreateInfo m_RenderPassCI;
		miru::Ref<miru::crossplatform::Framebuffer> m_Framebuffers[2];
		miru::crossplatform::Framebuffer::CreateInfo m_FramebufferCI;

		CreateInfo m_CI;
		int m_CurrentWidth, m_CurrentHeight;

	public:
		RenderSurface(CreateInfo* pCreateInfo);
		~RenderSurface();

		const CreateInfo& GetCreateInfo() { return m_CI; }
		
		void Resize(int width, int height);

		inline const miru::Ref<miru::crossplatform::Context> GetContext() const { return m_Context; };
		inline const miru::Ref<miru::crossplatform::Swapchain>& GetSwapchain() const { return m_Swapchain; };
		inline void* GetDevice() const { return m_Context->GetDevice(); }
		inline const miru::Ref<miru::crossplatform::RenderPass>& GetRenderPass() const { return m_RenderPass; }
		inline const miru::Ref<miru::crossplatform::ImageView>& GetSwapchainImageView(size_t index) const { return m_Swapchain->m_SwapchainImageViews[index]; }
		inline const miru::Ref<miru::crossplatform::ImageView>& GetSwapchainDepthImageView() const { return m_DepthImageView; }
		inline const miru::Ref<miru::crossplatform::Framebuffer>* GetFramebuffers() { return m_Framebuffers; }

		inline const miru::crossplatform::GraphicsAPI::API& GetGraphicsAPI() const { return m_CI.api; }
		inline bool IsD3D12() const { return miru::crossplatform::GraphicsAPI::IsD3D12(); }
		inline bool IsVulkan() const { return miru::crossplatform::GraphicsAPI::IsVulkan(); }
		inline int GetWidth() const { return m_CurrentWidth; }
		inline int GetHeight() const { return m_CurrentHeight; }
		inline float GetRatio() const { return ((float)m_CurrentWidth / (float)m_CurrentHeight); }
		inline bool& Resized() const { return m_Swapchain->m_Resized; }
		std::string GetGraphicsAPIVersion() const;
		std::string GetDeviceName() const;
		inline std::string GetAntiAliasingValue() const { return std::to_string(static_cast<uint32_t>(m_CI.samples)); }
	
	private:
		void CreateFramebuffer();
		friend class Window;
	};
}
}