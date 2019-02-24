#include "font.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;
using namespace ARM;
using namespace CROSSPLATFORM;

Font::Font(const char* filepath, int fontHeight, const Window& window)
	:m_FilePath(filepath), m_FontHeight(fontHeight), m_Window(window)
{
	GenerateCharacterMap();
}

Font::~Font()
{
	FT_Done_Face(m_Face);
	FT_Done_FreeType(m_ftlib);
}


void Font::AddLine(const std::string& text, const ARM::Vec2& position, const ARM::Vec4& colour)
{
	m_Lines.emplace_back(
	Line
	{
		text,
		position,
		position,
		colour
	});
	GenerateLine(m_Lines.size()-1);
}

void Font::Render()
{
	Mat4 proj = Mat4::Orthographic(0.0f, m_WindowWidth, 0.0f, m_WindowHeight, -1.0f, 1.0f);
	proj.Transpose();
	BufferManager::UpdateUBO(0, &proj.a, sizeof(Mat4), 0);

	if (b_GenerateRenderGlyphBuffer)
		GenerateRenderGlyphBuffer();

	m_FontRenderer.OpenMapBuffer();
	for(auto& glyph : m_RenderGlyphBuffer)
			m_FontRenderer.Submit(&glyph);
	m_FontRenderer.CloseMapBuffer();
	m_FontRenderer.Flush();
	
}

void Font::UpdateLine(const std::string& input, unsigned int lineIndex)
{
	if (lineIndex < m_Lines.size() && strcmp(m_Lines[lineIndex].m_Text.c_str(), input.c_str()))
	{
		m_Lines[lineIndex].m_Text = input;
		m_Lines[lineIndex].m_GlyphBuffer.clear();
		b_GenerateRenderGlyphBuffer = true;

		for (unsigned int i = 0; i < m_Lines.size(); i++)
		{
			GenerateLine(i);
				m_Lines[i].m_Position = m_Lines[i].m_InitPosition;
		}
	}
}

void Font::GenerateCharacterMap()
{
	FT_Library m_ftlib;
	if (FT_Init_FreeType(&m_ftlib))
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Font: Failed to initialise freetype.lib!" << std::endl;

	FT_Face m_Face;
	if (FT_New_Face(m_ftlib, m_FilePath, 0, &m_Face))
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Font: Failed to load font!" << std::endl;
	FT_Set_Pixel_Sizes(m_Face, 0, m_FontHeight);
	FT_Set_Char_Size(m_Face, 0, m_FontHeight * 16, 300, 300);

	//void LoadFont();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (int i = 0; i < GEAR_NUM_OF_CHARACTERS; i++)
	{
		if (FT_Error error = FT_Load_Char(m_Face, i, FT_LOAD_RENDER))
		{
			std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Font: Failed to load character: " << i << "! Error Code: 0x" << std::hex << error << std::dec << std::endl;;
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
}

void Font::GenerateLine(unsigned int lineIndex)
{
	m_Lines[lineIndex].m_GlyphBuffer.clear();
	std::string::const_iterator it;
	for (it = m_Lines[lineIndex].m_Text.begin(); it != m_Lines[lineIndex].m_Text.end(); it++)
	{
		Character ch = m_CharMap[*it];
		float scale = (float)m_FontHeight / 1000.0f;
		float pos_x = m_Lines[lineIndex].m_Position.x + ch.m_Bearing.x * scale;
		float pos_y = m_Lines[lineIndex].m_Position.y - (ch.m_Size.y - ch.m_Bearing.y) * scale;
		float width = ch.m_Size.x * scale;
		float height = ch.m_Size.y * scale;

		m_Lines[lineIndex].m_GlyphBuffer.emplace_back(CROSSPLATFORM::Object("res/obj/quad.obj", m_Shader, *ch.m_Texture, m_Lines[lineIndex].m_Colour, Vec3(pos_x, pos_y, 0.0f), Vec2(width, height)));
		m_Lines[lineIndex].m_Position.x += (ch.m_Advance >> 6) * m_WindowRatio * scale;
	}
}

void Font::GenerateRenderGlyphBuffer()
{
	m_RenderGlyphBuffer.clear();
	for (auto& line : m_Lines)
	{
		for (auto& glyph : line.m_GlyphBuffer)
			m_RenderGlyphBuffer.push_back(glyph);
		line.m_Position = line.m_InitPosition;
	}
	b_GenerateRenderGlyphBuffer = false;
}
