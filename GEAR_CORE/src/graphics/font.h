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
	const Window& m_Window;

	VertexArray* VAO;
	VertexBuffer* VBO;
	IndexBuffer* IBO;
	Texture* m_Texture;

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
	Font(const std::string& text, const char* filepath, int fontHeight, const ARM::Vec2& position, const ARM::Vec4& colour, const Window& window);
	~Font();
	void RenderText();

private:
	//void LoadFont();
	//void GenFontVAOVBO();
};
}
}