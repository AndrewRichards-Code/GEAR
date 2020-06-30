#include "font.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace mars;

using namespace miru;
using namespace miru::crossplatform;

Font::Font(void* device, const char* filepath, int fontHeight, int width, int height, float ratio)
	:m_Device(device), m_FilePath(filepath), m_FontHeight(fontHeight), m_WindowWidth(static_cast<float>(width)), m_WindowHeight(static_cast<float>(height)), m_WindowRatio(ratio)
{
	m_FontPipeline = std::make_shared<Pipeline>(m_Device, "res/shaders/bin/font.vert.spv", "res/shaders/bin/font.frag.spv");
	m_FontPipeline->SetViewport(0.0f, 0.0f, m_WindowWidth, m_WindowHeight, 0.0f, 1.0f);
	m_FontPipeline->SetRasterisationState(false, false, PolygonMode::FILL, CullModeBit::NONE_BIT, FrontFace::COUNTER_CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 0.0f);
	m_FontPipeline->SetMultisampleState(Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 0.0f, false, false);
	m_FontPipeline->SetDepthStencilState(false, false, CompareOp::ALWAYS, false, false, {}, {}, 0.0f, 1.0f);
	float blendConsts[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_FontPipeline->SetColourBlendState(false, LogicOp::CLEAR, {}, blendConsts);

	miru::crossplatform::RenderPass::CreateInfo fontRenderPassCI;
	fontRenderPassCI.debugName = "GEAR_CORE_FontRenderPass";
	fontRenderPassCI.device = m_Device;
	fontRenderPassCI.attachments = {
		{ 
			Image::Format::B8G8R8A8_UNORM, Image::SampleCountBit::SAMPLE_COUNT_1_BIT, RenderPass::AttachmentLoadOp::LOAD, RenderPass::AttachmentStoreOp::STORE,
			RenderPass::AttachmentLoadOp::DONT_CARE, RenderPass::AttachmentStoreOp::DONT_CARE, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
		}
	};
	fontRenderPassCI.subpassDescriptions = { {PipelineType::GRAPHICS,  {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {}, {}} };
	fontRenderPassCI.subpassDependencies = { {MIRU_SUBPASS_EXTERNAL, 0, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
		Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT,  Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT } };
	m_FontRenderPass = RenderPass::Create(&fontRenderPassCI);
	m_FontPipeline->SetRenderPass(m_FontRenderPass, 0);
	m_FontPipeline->FinalisePipline();
	GenerateCharacterMap();
}

Font::~Font()
{
	FT_Done_Face(m_Face);
	FT_Done_FreeType(m_ftlib);
}


void Font::AddLine(const std::string& text, const mars::Vec2& position, const mars::Vec4& colour)
{
	m_Lines.emplace_back(
	Line
	{
		text,
		position,
		position,
		colour
	});
	GenerateLine(static_cast<unsigned int>(m_Lines.size()-1));
}

void Font::Render()
{
	/*Mat4 proj = Mat4::Orthographic(0.0f, m_WindowWidth, 0.0f, m_WindowHeight, -1.0f, 1.0f);
	proj.Transpose();
	BufferManager::UpdateUBO(0, &proj.a, sizeof(Mat4), 0);

	if (b_GenerateRenderGlyphBuffer)
		GenerateRenderGlyphBuffer();

	m_FontRenderer.OpenMapBuffer();
	for(auto& glyph : m_RenderGlyphBuffer)
			m_FontRenderer.Submit(&glyph);
	m_FontRenderer.CloseMapBuffer();
	m_FontRenderer.Flush();*/
	
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
		std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to initialise freetype.lib!" << std::endl;

	FT_Face m_Face;
	if (FT_New_Face(m_ftlib, m_FilePath, 0, &m_Face))
		std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to load font!" << std::endl;
	FT_Set_Pixel_Sizes(m_Face, 0, m_FontHeight);
	FT_Set_Char_Size(m_Face, 0, m_FontHeight * 16, 300, 300);

	//void LoadFont();
	for (int i = 0; i < GEAR_NUM_OF_CHARACTERS; i++)
	{
		if (FT_Error error = FT_Load_Char(m_Face, i, FT_LOAD_RENDER))
		{
			std::cout << "ERROR: GEAR::GRAPHICS::Font: Failed to load character: " << i << "! Error Code: 0x" << std::hex << error << std::dec << std::endl;;
			continue;
		}

		Character character
		{
			std::make_shared<Texture>(m_Device, m_Face->glyph->bitmap.buffer, m_Face->glyph->bitmap.width, m_Face->glyph->bitmap.rows),
			Vec2((float)m_Face->glyph->bitmap.width, (float)m_Face->glyph->bitmap.rows),
			Vec2((float)m_Face->glyph->bitmap_left, (float)m_Face->glyph->bitmap_top),
			(unsigned int)m_Face->glyph->advance.x
		};

		m_Charmap[i] = character;
	}
}

void Font::GenerateLine(unsigned int lineIndex)
{
	m_Lines[lineIndex].m_GlyphBuffer.clear();
	std::string::const_iterator it;
	for (it = m_Lines[lineIndex].m_Text.begin(); it != m_Lines[lineIndex].m_Text.end(); it++)
	{
		Character ch = m_Charmap[*it];
		float scale = (float)m_FontHeight / 1000.0f;
		float pos_x = m_Lines[lineIndex].m_Position.x + ch.m_Bearing.x * scale;
		float pos_y = m_Lines[lineIndex].m_Position.y - (ch.m_Size.y - ch.m_Bearing.y) * scale;
		float width = ch.m_Size.x * scale;
		float height = ch.m_Size.y * scale;

		m_Lines[lineIndex].m_GlyphBuffer.emplace_back(OBJECTS::Object(m_Device, "res/obj/quad.obj", m_FontPipeline, ch.m_Texture, m_Lines[lineIndex].m_Colour, Vec3(pos_x, pos_y, 0.0f), Vec2(width, height)));
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
