#pragma once
#include "gear_common.h"
#include "GL/glew.h"

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
class Texture
{
public:
	enum class TextureType : unsigned int
	{
		GEAR_TEXTURE_UNKNOWN = 0,
		GEAR_TEXTURE_1D = GL_TEXTURE_1D,
		GEAR_TEXTURE_2D = GL_TEXTURE_2D,
		GEAR_TEXTURE_3D = GL_TEXTURE_3D,
		GEAR_TEXTURE_1D_ARRAY = GL_TEXTURE_1D_ARRAY,
		GEAR_TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY,
		GEAR_TEXTURE_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,
		GEAR_TEXTURE_2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
		GEAR_TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
		GEAR_TEXTURE_CUBE_MAP_ARRAY = GL_TEXTURE_CUBE_MAP_ARRAY,
	};
	enum class ImageFormat : unsigned int
	{
		GEAR_IMAGE_UNKNOWN = 0,
		GEAR_RGBA32F = GL_RGBA32F,												 //rgba32f
		GEAR_RGBA16F = GL_RGBA16F,												 //rgba16f
		GEAR_RG32F = GL_RG32F,													 //rg32f
		GEAR_RG16F = GL_RG16F,													 //rg16f
		GEAR_R11F_G11F_B10F = GL_R11F_G11F_B10F,								 //r11f_g11f_b10f
		GEAR_R32F = GL_R32F,													 //r32f
		GEAR_R16F = GL_R16F,													 //r16f
		GEAR_RGBA32UI = GL_RGBA32UI,											 //rgba32ui
		GEAR_RGBA16UI = GL_RGBA16UI,											 //rgba16ui
		GEAR_RGB10_A2UI = GL_RGB10_A2UI,										 //rgb10_a2ui
		GEAR_RGBA8UI = GL_RGBA8UI,												 //rgba8ui
		GEAR_RG32UI = GL_RG32UI,												 //rg32ui
		GEAR_RG16UI = GL_RG16UI,												 //rg16ui
		GEAR_RG8UI = GL_RG8UI,													 //rg8ui
		GEAR_R32UI = GL_R32UI,													 //r32ui
		GEAR_R16UI = GL_R16UI,													 //r16ui
		GEAR_R8UI = GL_R8UI,													 //r8ui
		GEAR_RGBA32I = GL_RGBA32I,												 //rgba32i
		GEAR_RGBA16I = GL_RGBA16I,												 //rgba16i
		GEAR_RGBA8I = GL_RGBA8I,												 //rgba8i
		GEAR_RG32I = GL_RG32I,													 //rg32i
		GEAR_RG16I = GL_RG16I,													 //rg16i
		GEAR_RG8I = GL_RG8I,													 //rg8i
		GEAR_R32I = GL_R32I,													 //r32i
		GEAR_R16I = GL_R16I,													 //r16i
		GEAR_R8I = GL_R8I,														 //r8i
		GEAR_RGBA16 = GL_RGBA16,												 //rgba16
		GEAR_RGB10_A2 = GL_RGB10_A2,											 //rgb10_a2
		GEAR_RGBA8 = GL_RGBA8,													 //rgba8
		GEAR_RG16 = GL_RG16,													 //rg16
		GEAR_RG8 = GL_RG8,														 //rg8
		GEAR_R16 = GL_R16,														 //r16
		GEAR_R8 = GL_R8,														 //r8
		GEAR_RED = GL_RED,														 //red
		GEAR_GREEN = GL_GREEN,													 //green
		GEAR_BLUE = GL_BLUE,													 //blue
		GEAR_ALPHA = GL_ALPHA,													 //aplha
		GEAR_RGBA16_SNORM = GL_RGBA16_SNORM,									 //rgba16_snorm
		GEAR_RGBA8_SNORM = GL_RGBA8_SNORM,										 //rgba8_snorm
		GEAR_RG16_SNORM = GL_RG16_SNORM,										 //rg16_snorm
		GEAR_RG8_SNORM = GL_RG8_SNORM,											 //rg8_snorm
		GEAR_R16_SNORM = GL_R16_SNORM,											 //r16_snorm
		GEAR_R8_SNORM = GL_R8_SNORM,											 //r8_snorm
		GEAR_DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,							 //depth_component16
		GEAR_DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,							 //depth_component24
		GEAR_DEPTH_COMPONENT32 = GL_DEPTH_COMPONENT32,							 //depth_component32
		GEAR_DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,						 //depth_component32f
		GEAR_DEPTH_STENCIL = GL_DEPTH_STENCIL,									 //depth_stencil
		GEAR_DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,							 //depth24_stencil8
		GEAR_DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,							 //depth32f_stencil8
		GEAR_UNSIGNED_INT_24_8 = GL_UNSIGNED_INT_24_8,							 //uint_24_8
		GEAR_FLOAT_32_UNSIGNED_INT_24_8_REV = GL_FLOAT_32_UNSIGNED_INT_24_8_REV  //float_32_uint24_8_rev
	};
	enum class BaseImageFormat
	{
		GEAR_BASE_IMAGE_UNKNOWN = 0,
		GEAR_RED = GL_RED,
		GEAR_RG = GL_RG,
		GEAR_RGB = GL_RGB,
		GEAR_RGBA = GL_RGBA,
		GEAR_DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
		GEAR_DEPTH_STENCIL = GL_DEPTH_STENCIL
	};
	BaseImageFormat ToBaseFormat(ImageFormat format)
	{
		switch (format)
		{
		default: return BaseImageFormat::GEAR_BASE_IMAGE_UNKNOWN;
		case ImageFormat::GEAR_IMAGE_UNKNOWN: return BaseImageFormat::GEAR_BASE_IMAGE_UNKNOWN;
		case ImageFormat::GEAR_RGBA32F: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RGBA16F: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RG32F: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_RG16F: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_R11F_G11F_B10F: return BaseImageFormat::GEAR_RGB;
		case ImageFormat::GEAR_R32F: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_R16F: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RGBA32UI: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RGBA16UI: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RGB10_A2UI: return BaseImageFormat::GEAR_RGB;
		case ImageFormat::GEAR_RGBA8UI: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RG32UI: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_RG16UI: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_RG8UI: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_R32UI: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_R16UI: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_R8UI: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_RGBA32I: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RGBA16I: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RGBA8I: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RG32I: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_RG16I: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_RG8I: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_R32I: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_R16I: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_R8I: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_RGBA16: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RGB10_A2: return BaseImageFormat::GEAR_RGB;
		case ImageFormat::GEAR_RGBA8: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RG16: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_RG8: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_R16: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_R8: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_RED: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_GREEN: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_BLUE: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_ALPHA: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_RGBA16_SNORM: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RGBA8_SNORM: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_RG16_SNORM: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_RG8_SNORM: return BaseImageFormat::GEAR_RG;
		case ImageFormat::GEAR_R16_SNORM: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_R8_SNORM: return BaseImageFormat::GEAR_RED;
		case ImageFormat::GEAR_DEPTH_COMPONENT16: return BaseImageFormat::GEAR_DEPTH_COMPONENT;
		case ImageFormat::GEAR_DEPTH_COMPONENT24: return BaseImageFormat::GEAR_DEPTH_COMPONENT;
		case ImageFormat::GEAR_DEPTH_COMPONENT32: return BaseImageFormat::GEAR_DEPTH_COMPONENT;
		case ImageFormat::GEAR_DEPTH_COMPONENT32F: return BaseImageFormat::GEAR_DEPTH_COMPONENT;
		case ImageFormat::GEAR_DEPTH_STENCIL: return BaseImageFormat::GEAR_DEPTH_STENCIL;
		case ImageFormat::GEAR_DEPTH24_STENCIL8: return BaseImageFormat::GEAR_DEPTH_STENCIL;
		case ImageFormat::GEAR_DEPTH32F_STENCIL8: return BaseImageFormat::GEAR_DEPTH_STENCIL;
		case ImageFormat::GEAR_UNSIGNED_INT_24_8: return BaseImageFormat::GEAR_RGBA;
		case ImageFormat::GEAR_FLOAT_32_UNSIGNED_INT_24_8_REV: return BaseImageFormat::GEAR_RGBA;
		}
	}

private:
	unsigned int m_TextureID;
	std::string m_FilePath;
	std::vector<std::string> m_FilePaths;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_Depth, m_BPP; //BPP = Bits per pixel
	TextureType m_Type = TextureType::GEAR_TEXTURE_UNKNOWN;
	ImageFormat m_Format = ImageFormat::GEAR_IMAGE_UNKNOWN;
	bool m_CubeMap = false;
	bool m_DepthTexture = false;
	float m_TileFactor = 1.0f;
	int m_Multisample = 1;
	static bool m_MipMapping;
	static bool m_AnisotrophicFilter;
	static float m_AnisotrophicValue;

public:
	Texture(const std::string& filepath); //Assumes GL_TEXTURE_2D!
	Texture(const std::vector<std::string>& filepaths); //For CubeMaps only! Submit in filepaths in order of: front, back, top, bottom, right and left.
	Texture(unsigned char* buffer, int width, int height); //For Fonts only!
	Texture(int width = 1024, int height = 1024, bool depthTexture = false); //For FrameBuffer only!
	Texture(TextureType type, ImageFormat format, int multisample = 1, int width = 0, int height = 0, int depth = 0); //General Texture constructor, suitable for compute shaders!
	Texture(TextureType type, std::vector<unsigned char*> buffer, ImageFormat format, int multisample = 1, int width = 0, int height = 0, int depth = 0); //General Texture constructor, suitable for compute shaders!
	Texture(TextureType type, const std::vector<std::string>& filepath, ImageFormat format, int multisample = 1, int width = 0, int height = 0, int depth = 0); //General Texture constructor, suitable for compute shaders!
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	void BindCubeMap(unsigned int slot = 0) const;
	void UnbindCubeMap() const;

	void Tile(float tileFactor);
	void Untile();
	static void EnableDisableMipMapping();
	static void EnableDisableAniostrophicFilting(float anisostrphicVal);

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline int GetBPP() const { return m_BPP; }
	inline unsigned int GetTextureID() const { return m_TextureID; }
	inline bool IsCubeMap() const { return m_CubeMap; }
	inline bool IsDepthTexture() const { return m_DepthTexture; }
	inline float GetTileFactor() const { return m_TileFactor; }
	inline static std::string GetAnisotrophicValue() { return std::to_string(static_cast<int>(m_AnisotrophicValue)); }

private:
	void AniostrophicFilting();
	void MipMapping();
};
}
}
}

/*if (glewIsSupported("GL_EXT_texture_filter_anisotropic") || GLEW_EXT_texture_filter_anisotropic)
{
	std::cout << "Anisotrophic Filter supported" << std::endl;
	float anisoMax;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoMax);
	std::cout << "Max Anisotrophic filtering: " << anisoMax << std::endl;
}*/