#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

Texture::Texture(const std::string& filepath)
	:m_FilePath(filepath), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
	stbi_set_flip_vertically_on_load(1);
	m_LocalBuffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);

	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);

	MipMapping();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	AniostrophicFilting();

	glBindTexture(GL_TEXTURE_2D, 0);

	if (m_LocalBuffer)
		stbi_image_free(m_LocalBuffer);
}
Texture::Texture(const std::vector<std::string>& filepaths)
	:m_FilePaths(filepaths), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0), m_CubeMap(true)
{	
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
		
	if (m_FilePaths.size() == 6)
	for (int i = 0; i < 6; i++)
	{
		stbi_set_flip_vertically_on_load(0);
		m_LocalBuffer = stbi_load(filepaths[i].c_str(), &m_Width, &m_Height, &m_BPP, 4);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
		
		if (m_LocalBuffer)
			stbi_image_free(m_LocalBuffer);
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Texture: 6 texture files have not been sunbmitted!" << std::endl;
	}

	MipMapping();
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	AniostrophicFilting();

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
}

Texture::Texture(unsigned char* buffer, int width, int height)
	:m_LocalBuffer(buffer), m_Width(width), m_Height(height), m_BPP(4)
{
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_Width, m_Height, 0, GL_RED, GL_UNSIGNED_BYTE, m_LocalBuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	AniostrophicFilting();

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(int width, int height, bool depthTexture)
	:m_Width(width), m_Height(height), m_DepthTexture(depthTexture)
{
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	
	if(m_DepthTexture)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_TextureID);
}

void Texture::Bind(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::BindCubeMap(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
}

void Texture::UnbindCubeMap() const
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Texture::Tile(float tileFactor)
{
	m_TileFactor = tileFactor;
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Untile()
{
	m_TileFactor = 1.0f;
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::m_MipMapping = false;
void Texture::EnableDisableMipMapping()
{
	if (m_MipMapping == true)
	{
		m_MipMapping = false;
	}
	else if (m_MipMapping == false)
	{
		m_MipMapping = true;
	}
}

void Texture::MipMapping() //Call in the constructor or bind texture beforehand.
{
	if (m_CubeMap == true)
	{
		if (m_MipMapping == true)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}
		else if (m_MipMapping == false)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}
	else if (m_CubeMap == false)
	{
		if (m_MipMapping == true)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else if (m_MipMapping == false)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}
}

bool Texture::m_AnisotrophicFilter = false;
float Texture::m_AnisotrophicValue = 1.0f;

void Texture::EnableDisableAniostrophicFilting(float anisostrphicVal)
{
	if (glewIsSupported("GL_EXT_texture_filter_anisotropic") || GLEW_EXT_texture_filter_anisotropic)
	{
		float anisoMax;
		//std::cout << "Anisotrophic Filter supported" << std::endl;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoMax);
		//std::cout << "Max Anisotrophic filtering: " << anisoMax << std::endl;
		if (anisostrphicVal <= anisoMax)
		{
			m_AnisotrophicValue = anisostrphicVal;
			if (m_AnisotrophicFilter == true)
			{	
				m_AnisotrophicFilter = false;
			}
			else if (m_AnisotrophicFilter == false)
			{
				m_AnisotrophicFilter = true;
			}
		}
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Texture: Anisotrophic Filter is not supported" << std::endl;
	}
}

void Texture::AniostrophicFilting() //Call in the constructor or bind texture beforehand.
{
	if (m_AnisotrophicFilter == true)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_AnisotrophicValue);
	}
	else if (m_AnisotrophicFilter == false)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
	}
}
