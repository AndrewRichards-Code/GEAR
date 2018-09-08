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
	Texture(const std::string& filepath);
	Texture(unsigned char* buffer, int width, int height); //For Fonts only!
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline int GetBPP() const { return m_BPP; }
	inline unsigned int GetTextureID() const { return m_TextureID; }
	inline unsigned char* GetLocalBuffer() const { return m_LocalBuffer; }
};
}
}