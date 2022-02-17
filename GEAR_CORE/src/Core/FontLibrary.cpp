#include "gear_core_common.h"
#include "STBI/stb_image.h"
#include "STBI/stb_image_write.h"

#include "FontLibrary.h"
#include "Graphics/AllocatorManager.h"
#include "Graphics/Texture.h"

using namespace gear;
using namespace graphics;
using namespace core;
using namespace mars;

using namespace miru;
using namespace miru::crossplatform;

FontLibrary::FontLibrary()
{
	if (FT_Init_FreeType(&m_FT_Lib))
	{
		GEAR_ASSERT(ErrorCode::OBJECTS | ErrorCode::INIT_FAILED, "Failed to initialise freetype.lib.");
	}
}
FontLibrary::~FontLibrary()
{
	FT_Done_FreeType(m_FT_Lib);
}

Ref<FontLibrary::Font> FontLibrary::LoadFont(LoadInfo* pLoadInfo)
{
	Ref<FontLibrary::Font> result;

	if (pLoadInfo->regenerateTextureAtlas)
	{
		result = GenerateFont(pLoadInfo->GI);
	}
	result = LoadGeneratedFont(pLoadInfo->GI);
	
	result->loadInfo = *pLoadInfo;
	return result;
}

Ref<FontLibrary::Font> FontLibrary::GenerateFont(const GenerateInfo& GI)
{
	FT_Face face;
	if (FT_New_Face(m_FT_Lib, GI.filepath.c_str(), 0, &face))
	{
		GEAR_ASSERT(ErrorCode::OBJECTS | ErrorCode::LOAD_FAILED, "Failed to load font.");
	}
	FT_Set_Pixel_Sizes(face, 0, GI.fontHeightPx);

	uint32_t fontHeight = static_cast<uint32_t>(face->size->metrics.height) / 64;
	uint32_t nunRows = GI.generatedTextureSize / fontHeight;

	std::vector<uint8_t> pixels;
	std::array<GlyphInfo, s_NumCaracters> glyphInfos;
	pixels.resize(GI.generatedTextureSize * GI.generatedTextureSize);
	uint32_t write_pos_x = 0;
	uint32_t write_pos_y = 0;

	for (uint32_t i = 0; i < s_NumCaracters; i++)
	{
		if (FT_Error error = FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			GEAR_WARN(ErrorCode::OBJECTS | ErrorCode::LOAD_FAILED, ("Failed to load character: " + std::to_string(i) + ". Error Code: " + std::to_string(error) + ".").c_str());
			continue;
		}
		const FT_Bitmap& bitmap = face->glyph->bitmap;

		//Start a new row
		if (write_pos_x + bitmap.width >= GI.generatedTextureSize)
		{
			write_pos_x = 0;
			write_pos_y += fontHeight + 1;
		}

		//Copy data
		for (uint32_t row = 0; row < bitmap.rows; row++)
		{
			for (uint32_t col = 0; col < bitmap.width; col++)
			{
				uint32_t x = write_pos_x + col;
				uint32_t y = write_pos_y + row;
				if (y * GI.generatedTextureSize + x < pixels.size())
				{
					pixels[y * GI.generatedTextureSize + x] = bitmap.buffer[row * bitmap.pitch + col];
				}
			}
		}

		//Fill out glyph info
		glyphInfos[i] = {
			write_pos_x, write_pos_y, static_cast<uint32_t>(bitmap.width), static_cast<uint32_t>(bitmap.rows),
			static_cast<uint32_t>(face->glyph->bitmap_left), static_cast<uint32_t>(face->glyph->bitmap_top),
			static_cast<uint32_t>(face->glyph->advance.x / 64) };

		write_pos_x += bitmap.width + 1;
	}

	std::vector<uint8_t> png_data;
	png_data.resize(GI.generatedTextureSize * GI.generatedTextureSize * 4);
	for (uint32_t i = 0; i < (GI.generatedTextureSize * GI.generatedTextureSize); i++)
	{
		png_data[i * 4 + 0] |= pixels[i];
		png_data[i * 4 + 1] |= pixels[i];
		png_data[i * 4 + 2] |= pixels[i];
		png_data[i * 4 + 3] = 0xff;
	}
	
	SaveGeneratedFont(png_data, glyphInfos, GI);

	Texture::DataTypeDataParameters data;
	data.data = png_data.data();
	data.size = png_data.size();
	data.width = GI.generatedTextureSize;
	data.height = GI.generatedTextureSize;
	data.depth = 1;

	Ref<Font>result = CreateRef<Font>();
	result->textureAtlas = GenerateTextureAtlas(GI, data);
	result->glyphInfos = glyphInfos;
	result->fontHeightPx = GI.fontHeightPx;
	return result;
}

Ref<FontLibrary::Font> FontLibrary::LoadGeneratedFont(const GenerateInfo& GI)
{
	Ref<Font>result = nullptr;

	std::string filepath = GI.filepath.substr(0, GI.filepath.find_last_of('.'));
	std::string filepathPNG = filepath + ".png";
	std::string filepathBIN = filepath + ".bin";

	if (std::filesystem::exists(filepathPNG) && std::filesystem::exists(filepathBIN))
	{
		result = CreateRef<Font>();
		Texture::DataTypeDataParameters data;

		std::ifstream stream(filepathBIN, std::ios::binary);
		if (stream.is_open())
		{
			uint32_t array_size;
			uint32_t zero = 0x00000000;
			stream.read((char*)&array_size, sizeof(uint32_t));
			stream.read((char*)&zero, sizeof(uint32_t));
			stream.read((char*)&zero, sizeof(uint32_t));
			stream.read((char*)&zero, sizeof(uint32_t));
			stream.read((char*)&zero, sizeof(uint32_t));
			stream.read((char*)&zero, sizeof(uint32_t));
			stream.read((char*)&zero, sizeof(uint32_t));
			stream.read((char*)&zero, sizeof(uint32_t));

			uint32_t i = 0;
			for (auto& glyphInfo : result->glyphInfos)
			{
				stream.read((char*)&i, sizeof(uint32_t));
				stream.read((char*)&glyphInfo.x, sizeof(uint32_t));
				stream.read((char*)&glyphInfo.y, sizeof(uint32_t));
				stream.read((char*)&glyphInfo.w, sizeof(uint32_t));
				stream.read((char*)&glyphInfo.h, sizeof(uint32_t));
				stream.read((char*)&glyphInfo.bearing_x, sizeof(uint32_t));
				stream.read((char*)&glyphInfo.bearing_y, sizeof(uint32_t));
				stream.read((char*)&glyphInfo.advance, sizeof(uint32_t));
			}

			uint32_t filepath_size;
			stream.read((char*)&filepath_size, sizeof(uint32_t));
			const char* _filepath = (const char*)alloca(filepath_size);
			stream.read((char*)_filepath, filepath_size);

			uint32_t bpp;
			data.data = stbi_load(filepathPNG.c_str(), (int*)&data.width, (int*)&data.height, (int*)&bpp, 4);
			data.size = data.width * data.height * 4;
			data.depth = 1;
			result->textureAtlas = GenerateTextureAtlas(GI, data);
			result->fontHeightPx = GI.fontHeightPx;

			stbi_image_free((void*)data.data);
		}
		stream.close();
	}
	else
	{
		result = GenerateFont(GI);
	}

	return result;
}

void FontLibrary::SaveGeneratedFont(const std::vector<uint8_t>& img_data, const std::array<GlyphInfo, s_NumCaracters>& glyphInfos, const GenerateInfo& GI)
{
	if (GI.savePNGandBINfiles)
	{
		std::string filepath = GI.filepath.substr(0, GI.filepath.find_last_of('.'));
		std::string filepathPNG = filepath + ".png";
		std::string filepathBIN = filepath + ".bin";

		stbi_write_png(filepathPNG.c_str(), GI.generatedTextureSize, GI.generatedTextureSize, 4, img_data.data(), GI.generatedTextureSize * 4);

		std::ofstream stream(filepathBIN, std::ios::binary);
		if (stream.is_open())
		{
			uint32_t array_size = static_cast<uint32_t>(glyphInfos.size()) * 8 * sizeof(uint32_t);
			uint32_t zero = 0x00000000;
			stream.write((char*)&array_size, sizeof(uint32_t));
			stream.write((char*)&zero, sizeof(uint32_t));
			stream.write((char*)&zero, sizeof(uint32_t));
			stream.write((char*)&zero, sizeof(uint32_t));
			stream.write((char*)&zero, sizeof(uint32_t));
			stream.write((char*)&zero, sizeof(uint32_t));
			stream.write((char*)&zero, sizeof(uint32_t));
			stream.write((char*)&zero, sizeof(uint32_t));

			uint32_t i = 0;
			for (auto& glyphInfo : glyphInfos)
			{
				stream.write((char*)&i, sizeof(uint32_t));
				stream.write((char*)&glyphInfo.x, sizeof(uint32_t));
				stream.write((char*)&glyphInfo.y, sizeof(uint32_t));
				stream.write((char*)&glyphInfo.w, sizeof(uint32_t));
				stream.write((char*)&glyphInfo.h, sizeof(uint32_t));
				stream.write((char*)&glyphInfo.bearing_x, sizeof(uint32_t));
				stream.write((char*)&glyphInfo.bearing_y, sizeof(uint32_t));
				stream.write((char*)&glyphInfo.advance, sizeof(uint32_t));
				i++;
			}

			uint32_t filepath_size = static_cast<uint32_t>(filepathPNG.size()) + 1;
			stream.write((char*)&filepath_size, sizeof(uint32_t));
			stream << filepathPNG;
			stream << '\0';
		}
		stream.close();
	}
}

Ref<Texture> FontLibrary::GenerateTextureAtlas(const GenerateInfo& GI, const Texture::DataTypeDataParameters& data)
{
	Texture::CreateInfo texCI;
	texCI.debugName = "GEAR_CORE_FontLibrary_Font_TextureAtlas: " + GI.filepath;
	texCI.device = AllocatorManager::GetCreateInfo().pContext->GetDevice();
	texCI.dataType = Texture::DataType::DATA;
	texCI.data = data;
	texCI.mipLevels = 1;
	texCI.arrayLayers = 1;
	texCI.type = Image::Type::TYPE_2D;
	texCI.format = Image::Format::R8G8B8A8_UNORM;
	texCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	texCI.usage = miru::crossplatform::Image::UsageBit(0);
	texCI.generateMipMaps = false;

	return CreateRef<Texture>(&texCI);
}
