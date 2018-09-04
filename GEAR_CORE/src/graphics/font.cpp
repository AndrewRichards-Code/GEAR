#include "font.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace ARM;

Font::Font(const std::string& text, const char* filepath, int fontHeight, const ARM::Vec2& position, const ARM::Vec4& colour, const Window& window)
	:m_Text(text), m_FilePath(filepath), m_FontHeight(fontHeight), m_Position(position), m_Colour(colour), m_Window(window)
{
	FT_Library m_ftlib;
	if (FT_Init_FreeType(&m_ftlib))
		std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to initialise freetype.lib!" << std::endl;

	FT_Face m_Face;
	if (FT_New_Face(m_ftlib, m_FilePath, 0, &m_Face))
		std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to load font!" << std::endl;
	FT_Set_Pixel_Sizes(m_Face, 0, m_FontHeight);


	//void LoadFont();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (GLubyte i = 0; i < 128; i++)
	{
		if (FT_Error error = FT_Load_Char(m_Face, i, FT_LOAD_RENDER))
		{
			std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to load character: " << i << "! Error Code: 0x" << std::hex << error << std::endl;;
			continue;
		}

		Character character
		{
			0,
			Vec2((float)m_Face->glyph->bitmap.width, (float)m_Face->glyph->bitmap.rows),
			Vec2((float)m_Face->glyph->bitmap_left, (float)m_Face->glyph->bitmap_top),
			(unsigned int)m_Face->glyph->advance.x
		};

		m_Texture = new Texture(character.m_FontTextID, m_Face->glyph->bitmap.buffer, (int)character.m_Size.x, (int)character.m_Size.y);
		m_CharMap.insert(std::pair<GLchar, Character>(i, character));
	
	}

	Mat4 proj = Mat4::Orthographic(-window.GetRatio(), window.GetRatio(), -1.0f, 1.0f, -1.0f, 1.0f);
	m_Shader.Enable();
	m_Shader.SetUniformMatrix<4>("u_Proj", 1, GL_TRUE, proj.a);
	m_Shader.SetUniform<float>("u_Colour", { m_Colour.r, m_Colour.g, m_Colour.b, m_Colour.a });
	m_Shader.Disable();
	
	FT_Done_Face(m_Face);
	FT_Done_FreeType(m_ftlib);

	//void GenFontVAOVBO();
	VAO = new VertexArray();
	VBO = new VertexBuffer(nullptr, 24, 4, GL_DYNAMIC_DRAW);
	VAO->AddBuffer(VBO, GEAR_BUFFER_POSITIONS);
	unsigned int indicies[]
	{ 0, 1, 2, 2, 3, 0 };
	IBO = new IndexBuffer(indicies, 6);
}

void Font::RenderText()
{
	m_Shader.Enable();
	m_Texture->Bind(0);
	VAO->Bind();

	std::string::const_iterator it;
	for (it = m_Text.begin(); it != m_Text.end(); it++)
	{
		Character ch = m_CharMap[*it];
		float scale = (float)m_FontHeight / 100.0f;
		float pos_x = m_Position.x + ch.m_Bearing.x * scale;
		float pos_y = m_Position.y - (ch.m_Size.y - ch.m_Bearing.y) * scale;
		float width = ch.m_Size.x * scale;
		float height = ch.m_Size.y * scale;

		float positions[24] =
		{
			pos_x,         pos_y + height,   0.0, 0.0,
			pos_x,         pos_y,            0.0, 1.0,
			pos_x + width, pos_y,            1.0, 1.0,

			pos_x,         pos_y + height,   0.0, 0.0,
			pos_x + width, pos_y,            1.0, 1.0,
			pos_x + width, pos_y + height,   1.0, 0.0
		};

		VBO->Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
		VBO->Unbind();
		
		m_Texture->Bind(0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Renderer fontRenderer;
		fontRenderer.Draw(*VAO, *IBO, m_Shader);
		m_Position.x += (ch.m_Advance >> 6) * m_FontHeight;
	}
	m_Texture->Unbind();
	VAO->Unbind();

}

Font::~Font()
{
	delete VAO;
	delete VBO;
	delete IBO;
	delete m_Texture;
}