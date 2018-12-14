#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "GL/glew.h"

namespace GEAR {
namespace GRAPHICS {
class Texture
{
private:
	unsigned int m_TextureID;
	std::string m_FilePath;
	std::vector<std::string> m_FilePaths;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP; //BPP = Bits per pixel
	bool m_CubeMap;
	bool m_DepthTexture;
	float m_TileFactor = 1.0f;
	static bool m_MipMapping;
	static bool m_AnisotrophicFilter;
	static float m_AnisotrophicValue;

public:
	Texture(const std::string& filepath); //Assumes GL_TEXTURE_2D!
	Texture(const std::vector<std::string>& filepaths); //For CubeMaps only! Submit in filepaths in order of: front, back, top, bottom, right and left.
	Texture(unsigned char* buffer, int width, int height); //For Fonts only!
	Texture(int width = 1024, int height = 1024, bool depthTexture = false); //For FrameBuffer only!
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

/*if (glewIsSupported("GL_EXT_texture_filter_anisotropic") || GLEW_EXT_texture_filter_anisotropic)
{
	std::cout << "Anisotrophic Filter supported" << std::endl;
	float anisoMax;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoMax);
	std::cout << "Max Anisotrophic filtering: " << anisoMax << std::endl;
}*/