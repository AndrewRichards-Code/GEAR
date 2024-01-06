#pragma once
#include "gear_core_common.h"
#include "Colour.h"

namespace gear 
{
	namespace graphics
	{
		class GEAR_API Texture
		{
		public:
			enum class DataType : uint32_t
			{
				DATA,
				FILE
			};

			struct DataTypeDataParameters
			{
				const uint8_t* data;
				size_t			size;
				uint32_t		width;
				uint32_t		height;
				uint32_t		depth;
			};
			struct DataTypeFileParameters
			{
				std::vector<std::string> filepaths;
			};

			//Provide either filepaths or data, size and image dimension details.
			struct CreateInfo
			{
				std::string							debugName;
				void* device;
				DataType							dataType;
				DataTypeDataParameters				data;
				DataTypeFileParameters				file;
				uint32_t							mipLevels;
				uint32_t							arrayLayers;
				miru::base::Image::Type				type;
				miru::base::Image::Format			format;
				miru::base::Image::SampleCountBit	samples;
				miru::base::Image::UsageBit			usage;
				bool								generateMipMaps;
				GammaSpace							gammaSpace;
			};
			uint32_t static constexpr MaxMipLevel = 16;

		private:
			miru::base::BufferRef m_ImageUploadBuffer;
			miru::base::BufferViewRef m_ImageUploadBufferView;

			miru::base::ImageRef m_Image;
			miru::base::ImageViewRef m_ImageView;

			miru::base::SamplerRef m_Sampler;

			CreateInfo m_CI;

			uint32_t m_Width = 0;
			uint32_t m_Height = 0;
			uint32_t m_Depth = 0;

			uint32_t m_BPP = 0; //BPP = Bytes per pixel
			bool m_HDR = false;
			bool m_Cubemap = false;
			bool m_DepthTexture = false;

		public:
			bool m_GenerateMipMaps = false;
			bool m_GeneratedMipMaps = false;


		public:
			Texture(CreateInfo* pCreateInfo);
			~Texture();

			CreateInfo& GetCreateInfo() { return m_CI; }

			void Reload();

			void SubmitImageData(std::vector<uint8_t>& imageData);
			void AccessImageData(std::vector<uint8_t>& imageData);

			inline const miru::base::BufferRef& GetCPUBuffer() const { return m_ImageUploadBuffer; }
			inline const miru::base::BufferViewRef& GetCPUBufferView() const { return m_ImageUploadBufferView; }
			inline const miru::base::ImageRef& GetImage() const { return m_Image; }
			inline const miru::base::ImageViewRef& GetImageView() const { return m_ImageView; }
			inline const miru::base::SamplerRef& GetSampler() const { return m_Sampler; }

			inline const uint32_t& GetWidth() const { return m_Width; }
			inline const uint32_t& GetHeight() const { return m_Height; }
			inline const uint32_t& GetDepth() const { return m_Depth; }

			inline bool IsCubemap() const { return m_Cubemap; }
			inline bool IsDepthTexture() const { return m_DepthTexture; }

			inline float GetAnisotrophicValue() const { return m_Sampler->GetCreateInfo().maxAnisotropy; };
			inline std::string GetAnisotrophicValue() { return std::to_string(static_cast<int>(m_Sampler->GetCreateInfo().maxAnisotropy)); }

			void SetSampler(miru::base::Sampler::CreateInfo* pSamplerCreateInfo);
			void SetSampler(miru::base::SamplerRef sampler);

		private:
			void CreateSampler();

			//This populates the parameter imageData. This should only be called in the constructor and Reload().
			void LoadImageData(std::vector<uint8_t>& imageData);
		};
	}
}