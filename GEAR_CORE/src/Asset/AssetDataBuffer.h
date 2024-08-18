#pragma once

#include "Asset.h"
#include "Core/DataBuffer.h"

namespace gear
{
	namespace asset
	{
		struct GEAR_API AssetDataBuffer final : public core::DataBuffer, public Asset
		{
			AssetDataBuffer() {}
			~AssetDataBuffer() {}
		};

		struct GEAR_API ImageAssetDataBuffer final : public core::DataBuffer, public Asset
		{
			ImageAssetDataBuffer() {}
			~ImageAssetDataBuffer() {}

			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t depth = 0;
			miru::base::Image::Format format = miru::base::Image::Format::UNKNOWN;
		};
	}
}