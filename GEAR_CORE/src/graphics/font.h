#pragma once

#include "GL/glew.h"
#include "ft2build.h"
#include FT_FREETYPE_H

#include "shader.h"
#include "texture.h"
#include "renderer/batchrenderer2d.h"
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
	ARM::Vec2 m_Position;
	ARM::Vec2 m_InitPosition;
	ARM::Vec4 m_Colour;
	const Window& m_Window;
	float m_WindowWidth = (float)m_Window.GetWidth();
	float m_WindowHeight = (float)m_Window.GetHeight();
	Shader m_Shader = Shader("res/shaders/font.vert", "res/shaders/font.frag");

	FT_Library m_ftlib;
	FT_Face m_Face;
	
	struct Character
	{
		Texture* m_Texture;
		ARM::Vec2 m_Size;
		ARM::Vec2 m_Bearing;
		unsigned int m_Advance;
	};
	std::map<GLchar, Character> m_CharMap;
	std::vector<Object> m_GlyphBuffer;

public:
	Font(const std::string& text, const char* filepath, int fontHeight, const ARM::Vec2& position, const ARM::Vec4& colour, const Window& window);
	~Font();
	void RenderText();
	void UpdateText(const std::string& input);

};
}
}