#include "font.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Font::Font(std::string text, const char* filepath, int fontHeight, Vec2 position, Vec4 colour, Window& window)
	:m_Text(text), m_FilePath(filepath), m_FontHeight(fontHeight), m_Position(position), m_Colour(colour), m_Window(window)
{
	FT_Library m_ftlib;
	if (FT_Init_FreeType(&m_ftlib))
		std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to initialise freetype.lib!" << std::endl;

	FT_Face m_Face;
	if (FT_New_Face(m_ftlib, m_FilePath, 0, &m_Face))
		std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to load font!" << std::endl;
	FT_Set_Pixel_Sizes(m_Face, 0, m_FontHeight);

	Mat4 proj = Mat4::Orthographic(-window.GetRatio(), window.GetRatio(), -1.0f, 1.0f, -1.0f, 1.0f);
	m_Shader.Enable();
	m_Shader.SetUniformMatrix<4>("u_Proj", 1, GL_TRUE, proj.a);
	m_Shader.SetUniform<float>("u_Colour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.Disable();

	//void LoadFont();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (GLubyte i = 0; i < 128; i++)
	{
		if (FT_Load_Char(m_Face, i, FT_LOAD_RENDER))
		{
			std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to load character: " << i << "!" << std::endl;
			continue;
		}
		
		Character character
		{
			0,
			Vec2((float)m_Face->glyph->bitmap.width, (float)m_Face->glyph->bitmap.rows),
			Vec2((float)m_Face->glyph->bitmap_left, (float)m_Face->glyph->bitmap_top),
			(unsigned int)m_Face->glyph->advance.x
		};
		
		Texture(character.m_FontTextID, m_Face->glyph->bitmap.buffer, (int)character.m_Size.x, (int)character.m_Size.y);
		m_CharMap.insert(std::pair<GLchar, Character>(i, character));
	}
	
	//void GenFontVAOVBO();
	VertexArray VAO;
	VertexBuffer VBO(nullptr, 24, 4, GL_DYNAMIC_DRAW);
	VAO.AddBuffer(&VBO, GEAR_BUFFER_POSITIONS);
	unsigned int indicies[]
	{ 0, 1, 2, 2, 3, 0 };
	IndexBuffer IBO(indicies, 6);
	
	
	//void RenderText();
	m_Shader.Enable();
	glActiveTexture(GL_TEXTURE0);
	VAO.Bind();

	std::string::const_iterator it;
	for (it = m_Text.begin(); it != m_Text.end(); it++)
	{
		Character ch = m_CharMap[*it];

		float pos_x = m_Position.x + ch.m_Bearing.x * m_FontHeight;
		float pos_y = m_Position.y - (ch.m_Size.y + ch.m_Bearing.y) * m_FontHeight;
		float width = ch.m_Size.x * m_FontHeight;
		float height = ch.m_Size.y * m_FontHeight;

		float positions[24] =
		{
			pos_x,         pos_y + height,   0.0, 0.0,
			pos_x,         pos_y,            0.0, 1.0,
			pos_x + width, pos_y,            1.0, 1.0,

			pos_x,         pos_y + height,   0.0, 0.0,
			pos_x + width, pos_y,            1.0, 1.0,
			pos_x + width, pos_y + height,   1.0, 0.0
		};

		VBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
		VBO.Unbind();
		
		glBindTexture(GL_TEXTURE_2D, ch.m_FontTextID);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Renderer fontRenderer;
		fontRenderer.Draw(VAO, IBO, m_Shader);
		m_Position.x += (ch.m_Advance >> 6) * m_FontHeight;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	VAO.Unbind();

	FT_Done_Face(m_Face);
	FT_Done_FreeType(m_ftlib);
}

Font::~Font()
{
}