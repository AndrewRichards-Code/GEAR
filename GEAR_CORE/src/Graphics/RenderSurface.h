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
			Ref<miru::crossplatform::Context>				pContext;
			uint32_t										width;
			uint32_t										height;
			miru::crossplatform::Image::SampleCountBit		samples;
		};

	private:
		Ref<miru::crossplatform::Allocator> m_AttachmentAllocator;

		//ColourSRGBImage
		Ref<miru::crossplatform::Image> m_ColourSRGBImage;
		miru::crossplatform::Image::CreateInfo m_ColourSRGBImageCI;
		Ref<miru::crossplatform::ImageView> m_ColourSRGBImageView;
		miru::crossplatform::ImageView::CreateInfo m_ColourSRGBImageViewCI;

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

		uint32_t m_CurrentWidth, m_CurrentHeight;
		const miru::crossplatform::Image::Format m_SRGBFormat = miru::crossplatform::Image::Format::B8G8R8A8_UNORM;
		const miru::crossplatform::Image::Format m_HDRFormat = miru::crossplatform::Image::Format::R16G16B16A16_SFLOAT;

	public:
		RenderSurface(CreateInfo* pCreateInfo);
		~RenderSurface();

		const CreateInfo& GetCreateInfo() { return m_CI; }
		
		void Resize(uint32_t width, uint32_t height);

		inline const Ref<miru::crossplatform::RenderPass>& GetMainRenderPass() const { return m_MainRenderPass; }
		inline const Ref<miru::crossplatform::RenderPass>& GetHDRRenderPass() const { return m_HDRRenderPass; }
		inline const Ref<miru::crossplatform::Framebuffer>* GetMainFramebuffers() { return m_MainFramebuffers; }
		inline const Ref<miru::crossplatform::Framebuffer>* GetHDRFramebuffers() { return m_HDRFramebuffers; }

		inline const Ref<miru::crossplatform::Context> GetContext() const { return m_CI.pContext; };
		inline void* GetDevice() const { return m_CI.pContext->GetDevice(); }
		inline uint32_t GetWidth() const { return m_CurrentWidth; }
		inline uint32_t GetHeight() const { return m_CurrentHeight; }
		inline float GetRatio() const { return ((float)m_CurrentWidth / (float)m_CurrentHeight); }
		inline std::string GetAntiAliasingValue() const { return std::to_string(static_cast<uint32_t>(m_CI.samples)); }
	
	private:
		void CreateAttachments();
		
		void CreateMainRenderPass();
		void CreateHDRRenderPass();

		void CreateMainFramebuffers();
		void CreateHDRFramebuffers();
		
		friend class Window;
	};
}
}