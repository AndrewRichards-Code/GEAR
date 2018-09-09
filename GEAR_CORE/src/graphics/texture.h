#pragma once
#include <string>
#include "GL/glew.h"

namespace GEAR {
namespace GRAPHICS {
class Texture
{
private:
	unsigned int m_TextureID;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP; //BPP = Bits per pixel

public:
	Texture(const std::string& filepath); //Assumes GL_TEXTURE_2D!
	Texture(const std::string& filepath, bool CubMap); //For CubeMaps only!
	Texture(unsigned char* buffer, int width, int height); //For Fonts only!
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	void BindCubeMap(unsigned int slot = 0) const;
	void UnbindCubeMap() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline int GetBPP() const { return m_BPP; }
	inline unsigned int GetTextureID() const { return m_TextureID; }
};
}
}