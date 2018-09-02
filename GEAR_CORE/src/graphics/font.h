#pragma once

#include "GL/glew.h"
#include "ft2build.h"
#include FT_FREETYPE_H


#include "shader.h"
#include "texture.h"
#include "renderer/renderer.h"
#include "window.h"

#include "../maths/ARMLib.h"

#include <iostream>
#include <map>

namespace GEAR {
namespace GRAPHICS {

class Font
{
private:
	std::string m_Text;
	const char* m_FilePath;
	int m_FontHeight;
	Shader m_Shader = Shader("res/shaders/font.vert", "res/shaders/font.frag");;
	ARM::Vec2 m_Position;
	ARM::Vec4 m_Colour;
	Window& m_Window;

	FT_Library m_ftlib;
	FT_Face m_Face;
	
	struct Character
	{
		unsigned int m_FontTextID;
		ARM::Vec2 m_Size;
		ARM::Vec2 m_Bearing;
		unsigned int m_Advance;
	};
	std::map<GLchar, Character> m_CharMap;

public:
	Font(std::string text, const char* filepath, int fontHeight, ARM::Vec2 position, ARM::Vec4 colour, Window& m_Window);
	~Font();

private:
	//void LoadFont();
	//void GenFontVAOVBO();
	//void RenderText();
};
}
}