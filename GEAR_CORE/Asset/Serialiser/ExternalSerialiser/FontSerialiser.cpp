#include "FontSerialiser.h"

#include "Asset/Serialiser/ExternalFileSerialiser.h"

#include "Graphics/AllocatorManager.h"
#include "Graphics/Texture.h"

#include "FREETYPE/include/ft2build.h"
#include FT_FREETYPE_H

#include <fstream>

using namespace gear;
using namespace asset;
using namespace serialiser;
using namespace graphics;
using namespace objects;

using namespace miru;
using namespace base;

FT_Library FontSerialiser::m_FT_Lib = nullptr;

static const uint32_t generatedTextureSize = 256;
static const uint32_t fontHeightPx = 16;

Ref<Asset> FontSerialiser::Deserialise(Asset::Handle handle, const AssetMetadata& metadata)
{
	if (m_FT_Lib == nullptr)
	{
		if (FT_Init_FreeType(&m_FT_Lib))
		{
			GEAR_FATAL(ErrorCode::ASSET | ErrorCode::INIT_FAILED, "Failed to initialise freetype.lib.");
		}
	}

	return LoadGeneratedFont(metadata);
}

void FontSerialiser::Serialise(Ref<Asset> asset, const AssetMetadata& metadata)
{
	GEAR_WARN(ErrorCode::ASSET | ErrorCode::NOT_SUPPORTED, "No support for Serialising Fonts.");
}

FontSerialiser::FontSerialiser()
{
	//TODO: fix me!
	__debugbreak();
}

FontSerialiser::~FontSerialiser()
{
	FT_Done_FreeType(m_FT_Lib);
}

Ref<Font> FontSerialiser::LoadGeneratedFont(const AssetMetadata& metadata)
{
	Ref<Font>result = nullptr;

	const std::filesystem::path& filepath = metadata.filepath.stem();
	const std::filesystem::path& filepathPNG = filepath / ".png";
	const std::filesystem::path& filepathBIN = filepath / ".bin";

	if (std::filesystem::exists(filepathPNG) && std::filesystem::exists(filepathBIN))
	{
		result = CreateRef<Font>();
		Ref<ImageAssetDataBuffer> imageDataBuffer = CreateRef<ImageAssetDataBuffer>();

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

			AssetMetadata a;
			a.type = Asset::Type::EXTERNAL_FILE;
			a.filepath = filepathPNG;
			imageDataBuffer = ref_cast<ImageAssetDataBuffer>(ExternalFileSerialiser::Deserialise(0, a));

			graphics::Texture::CreateInfo texCI;
			texCI.debugName = "GEAR_CORE_FontLibrary_Font_TextureAtlas: " + metadata.filepath.generic_string();
			texCI.device = graphics::AllocatorManager::GetCreateInfo().pContext->GetDevice();
			texCI.imageData = imageDataBuffer->Data;
			texCI.width = imageDataBuffer->width;
			texCI.height = imageDataBuffer->height;
			texCI.depth = imageDataBuffer->depth;
			texCI.mipLevels = 1;
			texCI.arrayLayers = 1;
			texCI.type = Image::Type::TYPE_2D;
			texCI.format = Image::Format::R8G8B8A8_UNORM;
			texCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
			texCI.usage = miru::base::Image::UsageBit(0);
			texCI.generateMipMaps = false;
			Ref<graphics::Texture> texture = CreateRef<graphics::Texture>(&texCI);

			result->textureAtlas = texture;
			result->fontHeightPx = fontHeightPx;
		}
		stream.close();
	}
	else
	{
		result = GenerateFont(metadata);
	}

	return result;
}

void FontSerialiser::SaveGeneratedFont(const Ref<asset::ImageAssetDataBuffer> imageDataBuffer, const std::array<Font::GlyphInfo, Font::s_NumCaracters>& glyphInfos, const AssetMetadata& metadata)
{
	const std::filesystem::path& filepath = metadata.filepath.stem();
	const std::filesystem::path& filepathPNG = filepath / ".png";
	const std::filesystem::path& filepathBIN = filepath / ".bin";

	AssetMetadata a;
	a.type = Asset::Type::EXTERNAL_FILE;
	a.filepath = filepathPNG;
	ExternalFileSerialiser::Serialise(imageDataBuffer, a);

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

		uint32_t filepath_size = static_cast<uint32_t>(filepathPNG.generic_string().length()) + 1;
		stream.write((char*)&filepath_size, sizeof(uint32_t));
		stream << filepathPNG;
		stream << '\0';
	}
	stream.close();
}

Ref<Font> FontSerialiser::GenerateFont(const AssetMetadata& metadata)
{
	FT_Face face;
	if (FT_New_Face(m_FT_Lib, metadata.filepath.generic_string().c_str(), 0, &face))
	{
		GEAR_FATAL(ErrorCode::OBJECTS | ErrorCode::LOAD_FAILED, "Failed to load font.");
	}


	FT_Set_Pixel_Sizes(face, 0, fontHeightPx);

	uint32_t fontHeight = static_cast<uint32_t>(face->size->metrics.height) / 64;

	std::vector<uint8_t> pixels;
	std::array<Font::GlyphInfo, Font::s_NumCaracters> glyphInfos;
	pixels.resize(generatedTextureSize * generatedTextureSize);
	uint32_t write_pos_x = 0;
	uint32_t write_pos_y = 0;

	for (uint32_t i = 0; i < Font::s_NumCaracters; i++)
	{
		if (FT_Error error = FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			GEAR_WARN(ErrorCode::OBJECTS | ErrorCode::LOAD_FAILED, ("Failed to load character: " + std::to_string(i) + ". Error Code: " + std::to_string(error) + ".").c_str());
			continue;
		}
		const FT_Bitmap& bitmap = face->glyph->bitmap;

		//Start a new row
		if (write_pos_x + bitmap.width >= generatedTextureSize)
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
				if (y * generatedTextureSize + x < pixels.size())
				{
					pixels[y * generatedTextureSize + x] = bitmap.buffer[row * bitmap.pitch + col];
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

	const uint32_t& width = generatedTextureSize;
	const uint32_t& height = write_pos_y + fontHeight + 1;

	Ref<ImageAssetDataBuffer> imageDataBuffer = CreateRef<ImageAssetDataBuffer>();
	imageDataBuffer->width = width;
	imageDataBuffer->height = height;
	imageDataBuffer->depth = 1;
	imageDataBuffer->format = Image::Format::R8G8B8A8_UNORM;

	std::vector<uint8_t>& imageData = imageDataBuffer->Data;
	imageData.resize(width * height * 4);
	for (uint32_t i = 0; i < width * height; i++)
	{
		imageData[i * 4 + 0] |= pixels[i];
		imageData[i * 4 + 1] |= pixels[i];
		imageData[i * 4 + 2] |= pixels[i];
		imageData[i * 4 + 3] = 0xff;
	}

	SaveGeneratedFont(imageDataBuffer, glyphInfos, metadata);

	graphics::Texture::CreateInfo texCI;
	texCI.debugName = "GEAR_CORE_FontLibrary_Font_TextureAtlas: " + metadata.filepath.generic_string();
	texCI.device = graphics::AllocatorManager::GetCreateInfo().pContext->GetDevice();
	texCI.imageData = imageDataBuffer->Data;
	texCI.width = imageDataBuffer->width;
	texCI.height = imageDataBuffer->height;
	texCI.depth = imageDataBuffer->depth;
	texCI.mipLevels = 1;
	texCI.arrayLayers = 1;
	texCI.type = Image::Type::TYPE_2D;
	texCI.format = Image::Format::R8G8B8A8_UNORM;
	texCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	texCI.usage = miru::base::Image::UsageBit(0);
	texCI.generateMipMaps = false;
	Ref<graphics::Texture> texture = CreateRef<graphics::Texture>(&texCI);

	Ref<Font>result = CreateRef<Font>();
	result->textureAtlas = texture;
	result->glyphInfos = glyphInfos;
	result->fontHeightPx = fontHeightPx;
	return result;
}
