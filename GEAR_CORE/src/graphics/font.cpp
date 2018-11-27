#include "font.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Font::Font(const std::string& text, const char* filepath, int fontHeight, const ARM::Vec2& position, const ARM::Vec4& colour, const Window& window)
	:m_Text(text), m_FilePath(filepath), m_FontHeight(fontHeight), m_Position(position), m_Colour(colour), m_Window(window)
{
	m_InitPosition = m_Position;

	FT_Library m_ftlib;
	if (FT_Init_FreeType(&m_ftlib))
		std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to initialise freetype.lib!" << std::endl;

	FT_Face m_Face;
	if (FT_New_Face(m_ftlib, m_FilePath, 0, &m_Face))
		std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to load font!" << std::endl;
	FT_Set_Pixel_Sizes(m_Face, 0, m_FontHeight);
	FT_Set_Char_Size(m_Face, 0, m_FontHeight * 16, 300, 300);


	//void LoadFont();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (int i = 0; i < 128; i++)
	{
		if (FT_Error error = FT_Load_Char(m_Face, i, FT_LOAD_RENDER))
		{
			std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to load character: " << i << "! Error Code: 0x" << std::hex << error << std::endl;;
			continue;
		}
		
		Character character
		{
			std::make_shared<Texture>(m_Face->glyph->bitmap.buffer, m_Face->glyph->bitmap.width, m_Face->glyph->bitmap.rows),
			Vec2((float)m_Face->glyph->bitmap.width, (float)m_Face->glyph->bitmap.rows),
			Vec2((float)m_Face->glyph->bitmap_left, (float)m_Face->glyph->bitmap_top),
			(unsigned int)m_Face->glyph->advance.x
		};
		
		m_CharMap.insert(std::pair<GLchar, Character>(i, character));
	}
	
	std::string::const_iterator it;
	for (it = m_Text.begin(); it != m_Text.end(); it++)
	{
		Character ch = m_CharMap[*it];
		float scale = (float)m_FontHeight / 1000.0f;
		float pos_x = m_Position.x + ch.m_Bearing.x * scale;
		float pos_y = m_Position.y - (ch.m_Size.y - ch.m_Bearing.y) * scale;
		float width = ch.m_Size.x * scale;
		float height = ch.m_Size.y * scale;

		m_GlyphBuffer.emplace_back(Object("res/obj/quad.obj", m_Shader, *ch.m_Texture, Vec3(pos_x, pos_y, 0.0f), Vec2(width, height)));
		m_Position.x += (ch.m_Advance >> 6) * m_WindowRatio * scale;
	}
	
	Mat4 proj = Mat4::Orthographic(0.0f, m_WindowWidth, 0.0f, m_WindowHeight, -1.0f, 1.0f);
	m_Shader.Enable();
	m_Shader.SetUniformMatrix<4>("u_Proj", 1, GL_TRUE, proj.a);
	m_Shader.SetUniform<float>("u_Colour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.Disable();
}

void Font::RenderText()
{
	m_FontRenderer.OpenMapBuffer();
	for (int i = 0; i < (int)m_GlyphBuffer.size(); i++)
	{
		m_FontRenderer.Submit(&m_GlyphBuffer[i]);
	}
	m_FontRenderer.CloseMapBuffer();
	
	m_FontRenderer.Flush();
	m_Position = m_InitPosition;
}

void Font::UpdateText(const std::string & input)
{
	m_Text = input;
	m_GlyphBuffer.clear();

	std::string::const_iterator it;
	for (it = m_Text.begin(); it != m_Text.end(); it++)
	{
		Character ch = m_CharMap[*it];
		float scale = (float)m_FontHeight / 1000.0f;
		float pos_x = m_Position.x + ch.m_Bearing.x * scale;
		float pos_y = m_Position.y - (ch.m_Size.y - ch.m_Bearing.y) * scale;
		float width = ch.m_Size.x * scale;
		float height = ch.m_Size.y * scale;

		m_GlyphBuffer.emplace_back(Object("res/obj/quad.obj", m_Shader, *ch.m_Texture, Vec3(pos_x, pos_y, 0.0f), Vec2(width, height)));
		m_Position.x += (ch.m_Advance >> 6) * m_WindowRatio * scale;
	}
}

Font::~Font()
{
	FT_Done_Face(m_Face);
	FT_Done_FreeType(m_ftlib);
}