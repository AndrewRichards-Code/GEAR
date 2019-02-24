#pragma once

#include "GL/glew.h"
#include "ft2build.h"
#include FT_FREETYPE_H

#include "shader/shader.h"
#include "texture.h"
#include "renderer/batchrenderer2d.h"
#include "window.h"

#include "../../maths/ARMLib.h"

#include <iostream>
#include <map>
#define GEAR_NUM_OF_CHARACTERS 128

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
class Font
{
private:
	const char* m_FilePath;
	int m_FontHeight;
	const Window& m_Window;
	float m_WindowWidth = static_cast<float>(m_Window.GetWidth());
	float m_WindowHeight = static_cast<float>(m_Window.GetHeight());
	float m_WindowRatio = static_cast<float>(m_Window.GetRatio());
	Shader m_Shader = Shader("res/shaders/GLSL/font.vert", "res/shaders/GLSL/font.frag");
	BatchRenderer2D m_FontRenderer;

	FT_Library m_ftlib;
	FT_Face m_Face;

private:
	struct Line
	{
		std::string m_Text;
		ARM::Vec2 m_InitPosition;
		ARM::Vec2 m_Position;
		ARM::Vec4 m_Colour;
		std::vector<CROSSPLATFORM::Object> m_GlyphBuffer;
	};
	std::vector<Line> m_Lines;

	struct Character
	{
		std::shared_ptr<Texture> m_Texture;
		ARM::Vec2 m_Size;
		ARM::Vec2 m_Bearing;
		unsigned int m_Advance;
	};
	std::map<GLchar, Character> m_CharMap;

	bool b_GenerateRenderGlyphBuffer = true;
	std::vector<CROSSPLATFORM::Object> m_RenderGlyphBuffer;

public:
	Font(const char* filepath, int fontHeight, const Window& window);
	~Font();
	void AddLine(const std::string& text, const ARM::Vec2& position, const ARM::Vec4& colour);
	void Render();
	void UpdateLine(const std::string& input, unsigned int lineIndex);

private:
	void GenerateCharacterMap();
	void GenerateLine(unsigned int lineIndex);
	void GenerateRenderGlyphBuffer();
};
}
}
}