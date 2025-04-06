#pragma once
#include "Asset/Asset.h"

namespace gear
{
	namespace graphics
	{
		class Texture;
	}
	namespace objects
	{
		class GEAR_OBJECTS_API Font : public asset::Asset
		{
			//enum/struct
		public:
			struct GlyphInfo
			{
				uint32_t x, y, w, h;
				uint32_t bearing_x, bearing_y;
				uint32_t advance;
			};

			constexpr static size_t s_NumCaracters = 512;

			//Members
		public:
			Ref<graphics::Texture>					textureAtlas;
			std::array<GlyphInfo, s_NumCaracters>	glyphInfos;
			uint32_t								fontHeightPx;
		};
	}
}