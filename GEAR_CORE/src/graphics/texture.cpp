#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace GEAR;
using namespace GRAPHICS;

Texture::Texture(const std::string& filepath)
	:m_FilePath(filepath), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
	stbi_set_flip_vertically_on_load(1);
	m_LocalBuffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);

	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (m_LocalBuffer)
		stbi_image_free(m_LocalBuffer);
}
Texture::Texture(const std::vector<std::string>& filepaths)
	:m_FilePaths(filepaths), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0), m_CubeMap(true)
{	

	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
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
		std::cout << "ERROR: GEAR::GRAPHIC::Texture: 6 texture file have not been sunbmitted!" << std::endl;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
}

Texture::Texture(unsigned char* buffer, int width, int height)
	:m_LocalBuffer(buffer), m_Width(width), m_Height(height), m_BPP(4)
{
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_Width, m_Height, 0, GL_RED, GL_UNSIGNED_BYTE, m_LocalBuffer);
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