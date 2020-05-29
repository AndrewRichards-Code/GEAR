#pragma once

#include "gear_core_common.h"

#include "pipeline.h"
#include "texture.h"
#include "renderer/batchrenderer2d.h"

#include "mars.h"

#define GEAR_NUM_OF_CHARACTERS 128

namespace GEAR {
namespace GRAPHICS {
class Font
{
private:
	void* m_Device;

	const char* m_FilePath;
	int m_FontHeight;
	float m_WindowWidth;
	float m_WindowHeight;
	float m_WindowRatio;
	miru::Ref<miru::crossplatform::RenderPass> m_FontRenderPass;
	std::unique_ptr<Pipeline> m_FontPipeline;
	BatchRenderer2D m_FontRenderer;

	FT_Library m_ftlib;
	FT_Face m_Face;

private:
	struct Line
	{
		std::string m_Text;
		mars::Vec2 m_InitPosition;
		mars::Vec2 m_Position;
		mars::Vec4 m_Colour;
		std::vector<OBJECTS::Object> m_GlyphBuffer;
	};
	std::vector<Line> m_Lines;

	struct Character
	{
		std::shared_ptr<Texture> m_Texture;
		mars::Vec2 m_Size;
		mars::Vec2 m_Bearing;
		unsigned int m_Advance;
	};
	std::map<char, Character> m_Charmap;

	bool b_GenerateRenderGlyphBuffer = true;
	std::vector<OBJECTS::Object> m_RenderGlyphBuffer;

public:
	Font(void* device, const char* filepath, int fontHeight, int width, int height, float ratio);
	~Font();
	void AddLine(const std::string& text, const mars::Vec2& position, const mars::Vec4& colour);
	void Render();
	void UpdateLine(const std::string& input, unsigned int lineIndex);

private:
	void GenerateCharacterMap();
	void GenerateLine(unsigned int lineIndex);
	void GenerateRenderGlyphBuffer();
};
}
}