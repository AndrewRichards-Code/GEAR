#pragma once

#include "gear_core_common.h"
#include "Graphics/Texture.h"

namespace gear
{
namespace graphics
{
	class Texture;
}

namespace objects
{
	class FontLibrary
	{
	private:
		constexpr static size_t s_NumCaracters = 512;

	public:
		struct GenerateInfo
		{
			std::string filepath;
			uint32_t	fontHeightPx;
			uint32_t	generatedTextureSize;
			bool		savePNGandBINfiles;
		};

		struct LoadInfo
		{
			GenerateInfo	GI;
			bool			regenerateTextureAtlas;
		};
		
		struct GlyphInfo
		{
			uint32_t x, y, w, h;
			uint32_t bearing_x, bearing_y;
			uint32_t advance;
		};

		struct Font
		{
			Ref<graphics::Texture>			textureAtlas;
			std::array<GlyphInfo, s_NumCaracters>	glyphInfos;
			uint32_t								fontHeightPx;
		};

	private:
		FT_Library m_FT_Lib;

	public:
		FontLibrary();
		~FontLibrary();

		Ref<Font> LoadFont(LoadInfo* pLoadInfo);

	private:
		Ref<Font> GenerateFont(const GenerateInfo& GI);
		Ref<Font> LoadGeneratedFont(const GenerateInfo& GI);
		void SaveGeneratedFont(const std::vector<uint8_t>& img_data, const std::array<GlyphInfo, s_NumCaracters>& glyphInfos, const GenerateInfo& GI);
		Ref<graphics::Texture> GenerateTextureAtlas(const GenerateInfo& GI, const graphics::Texture::DataTypeDataParameters& data);
	};
}
}
