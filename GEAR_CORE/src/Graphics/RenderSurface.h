#pragma once

#include "gear_core_common.h"

namespace gear 
{
	namespace graphics
	{
		class GEAR_API RenderSurface
		{
		public:
			struct CreateInfo
			{
				std::string								debugName;
				miru::base::ContextRef					pContext;
				uint32_t								width;
				uint32_t								height;
				miru::base::Image::SampleCountBit		samples;
			};

		private:
			miru::base::AllocatorRef m_AttachmentAllocator;

			//ColourSRGBImage
			miru::base::ImageRef m_ColourSRGBImage;
			miru::base::ImageViewRef m_ColourSRGBImageView;

			//DepthImage
			miru::base::ImageRef m_DepthImage;
			miru::base::ImageViewRef m_DepthImageView;

			//MSAAColourImage
			miru::base::ImageRef m_MSAAColourImage;
			miru::base::ImageViewRef m_MSAAColourImageView;

			//ColourImage
			miru::base::ImageRef m_ColourImage;
			miru::base::ImageViewRef m_ColourImageView;

			CreateInfo m_CI;

			uint32_t m_CurrentWidth, m_CurrentHeight;

		public:
			static constexpr miru::base::Image::Format SRGBFormat = miru::base::Image::Format::B8G8R8A8_UNORM;
			static constexpr miru::base::Image::Format HDRFormat = miru::base::Image::Format::R16G16B16A16_SFLOAT;
			static constexpr miru::base::Image::Format DepthFormat = miru::base::Image::Format::D32_SFLOAT;

		public:
			RenderSurface(CreateInfo* pCreateInfo);
			~RenderSurface();

			const CreateInfo& GetCreateInfo() { return m_CI; }

			void Resize(uint32_t width, uint32_t height);

			inline const miru::base::ImageViewRef& GetColourSRGBImageView() const { return m_ColourSRGBImageView; }
			inline const miru::base::ImageViewRef& GetDepthImageView() const { return m_DepthImageView; }
			inline const miru::base::ImageViewRef& GetMSAAColourImageView() { return m_MSAAColourImageView; }
			inline const miru::base::ImageViewRef& GetColourImageView() { return m_ColourImageView; }

			inline const miru::base::ContextRef GetContext() const { return m_CI.pContext; };
			inline void* GetDevice() const { return m_CI.pContext->GetDevice(); }
			inline uint32_t GetWidth() const { return m_CurrentWidth; }
			inline uint32_t GetHeight() const { return m_CurrentHeight; }
			inline float GetRatio() const { return ((float)m_CurrentWidth / (float)m_CurrentHeight); }
			inline const miru::base::Image::SampleCountBit& GetAntiAliasing() const { return m_CI.samples; }
			inline std::string GetAntiAliasingValue() const { return std::to_string(static_cast<uint32_t>(m_CI.samples)); }

		private:
			void CreateAttachments();

			friend class Window;
		};
	}
}