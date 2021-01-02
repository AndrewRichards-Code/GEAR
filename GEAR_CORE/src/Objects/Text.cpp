#include "gear_core_common.h"
#include "Text.h"

using namespace gear;
using namespace graphics;
using namespace objects;
using namespace mars;

using namespace miru;
using namespace miru::crossplatform;

Text::Text(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	Update();
}

Text::~Text()
{
}

void Text::AddLine(const gear::Ref<FontLibrary::Font>& font, const std::string& text, const Uint2& position, const Vec4& colour, const Vec4& backgroundColour)
{
	Uint2 _position = position;
	_position.y += font->fontHeightPx;
	m_Lines.push_back({ font, text, _position, _position , colour, backgroundColour, {} });
	GenerateLine(m_Lines.size() - 1);
}

void Text::UpdateLine(const std::string& text, size_t lineIndex, bool force)
{
	if (lineIndex < m_Lines.size() && force)
	{
		Line& line = m_Lines[lineIndex];
		if (strcmp(line.text.c_str(), text.c_str()))
		{
			line.text = text;
			line.position = line.initPosition;
			GenerateLine(lineIndex, true);
		}
	}
}

void Text::Update()
{
	m_CameraCI.debugName = "GEAR_CORE_FontRenderer";
	m_CameraCI.device = m_CI.device;
	m_CameraCI.projectionType = Camera::ProjectionType::ORTHOGRAPHIC;
	m_CameraCI.orthographicsParams = { 0.0f, static_cast<float>(m_CI.viewportWidth), 0.0f, static_cast<float>(m_CI.viewportHeight), 1.0f, -1.0f }; //TODO: Account for reverse depth.
	m_CameraCI.flipX = false;
	m_CameraCI.flipY = false;
	m_Camera = gear::CreateRef<Camera>(&m_CameraCI);
}

void Text::GenerateLine(size_t lineIndex, bool update)
{
	Line& line = m_Lines[lineIndex];
	float viewportRatio = static_cast<float>(m_CI.viewportWidth) / static_cast<float>(m_CI.viewportHeight);
	const gear::Ref<FontLibrary::Font>& font = line.font;
	uint32_t background_height = font->fontHeightPx + 4;

	Material::CreateInfo fontMaterialCI;
	fontMaterialCI.debugName = "GEAR_CORE_FontRenderer: " + font->textureAtlas->GetCreateInfo().debugName;
	fontMaterialCI.device = m_CI.device;
	fontMaterialCI.pbrTextures = { { Material::TextureType::ALBEDO, font->textureAtlas } };

	Mesh::CreateInfo textMeshCI;
	textMeshCI.debugName = "GEAR_CORE_FontRenderer: " + line.text;
	textMeshCI.device = m_CI.device;
	textMeshCI.filepath = "";

	ModelLoader::ModelData& modelData = textMeshCI.data;
	modelData.push_back({});

	ModelLoader::MeshData& meshData = modelData.back();
	meshData.name = line.text;

	if (!update)
		meshData.pMaterial = gear::CreateRef<Material>(&fontMaterialCI);
	else
		meshData.pMaterial = line.model->m_CI.pMesh->m_CI.data.back().pMaterial; //Grab old one!

	//Make space for background quad
	meshData.vertices.push_back({});
	meshData.vertices.push_back({});
	meshData.vertices.push_back({});
	meshData.vertices.push_back({});
	meshData.indices.push_back(0);
	meshData.indices.push_back(1);
	meshData.indices.push_back(2);
	meshData.indices.push_back(2);
	meshData.indices.push_back(1);
	meshData.indices.push_back(3);
	
	//Characters
	uint32_t charIndex = 1;
	std::string::const_iterator it;
	for (it = line.text.begin(); it != line.text.end(); it++)
	{
		FontLibrary::GlyphInfo gi = font->glyphInfos[static_cast<size_t>(*it)];
		
		if (*it == '\n')
		{
			line.position.x = line.initPosition.x;
			line.position.y += gi.h;
			continue;
		}

		float pos_x = static_cast<float>(line.position.x + gi.bearing_x);
		float pos_y = static_cast<float>(line.position.y - (gi.h - gi.bearing_y));
		if (static_cast<float>(line.position.y) > pos_y)
		{
			pos_y += 2.0f * (static_cast<float>(line.position.y) - pos_y);
		}
		pos_y *= -1.0f;
		if (GraphicsAPI::IsD3D12())
			pos_y += static_cast<float>(m_CI.viewportHeight);

		float x = static_cast<float>(gi.x);
		float y = static_cast<float>(gi.y);
		float w = static_cast<float>(gi.w);
		float h = static_cast<float>(gi.h);
		float textureSize = static_cast<float>(font->textureAtlas->GetWidth());

		for (size_t i = 0; i < 4; i++)
		{
			ModelLoader::Vertex vertex;
			switch (i)
			{
			case 0:
			{
				vertex.position = Vec4(pos_x + 0.0f, pos_y + h, 0.0f, 1.0f);
				vertex.texCoord = Vec2(x + 0.0f, y + 0.0f) * (1.0f / textureSize);
				break;
			}
			case 1:
			{
				vertex.position = Vec4(pos_x + 0.0f, pos_y + 0.0f, 0.0f, 1.0f);
				vertex.texCoord = Vec2(x + 0.0f, y + h) * (1.0f / textureSize);
				break;
			}
			case 2:
			{
				vertex.position = Vec4(pos_x + w, pos_y + h, 0.0f, 1.0f);
				vertex.texCoord = Vec2(x + w, y + 0.0f) * (1.0f / textureSize);
				break;
			}
			case 3:
			{
				vertex.position = Vec4(pos_x + w, pos_y + 0.0f, 0.0f, 1.0f);
				vertex.texCoord = Vec2(x + w, y + h) * (1.0f / textureSize);
				break;
			}
			}
			vertex.normal = Vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vertex.tangent = Vec4(1.0f, 0.0f, 0.0f, 0.0f);
			vertex.binormal = Vec4(1.0f, 0.0f, 0.0f, 0.0f);
			vertex.colour = line.colour;
			meshData.vertices.push_back(vertex);
		}
		
		meshData.indices.push_back(0 + (4 * charIndex));
		meshData.indices.push_back(1 + (4 * charIndex));
		meshData.indices.push_back(2 + (4 * charIndex));
		meshData.indices.push_back(2 + (4 * charIndex));
		meshData.indices.push_back(1 + (4 * charIndex));
		meshData.indices.push_back(3 + (4 * charIndex));
		charIndex++;

		line.position.x += gi.advance;
	}

	//Background
	for (uint32_t i = 0; i < 4; i++)
	{
		ModelLoader::Vertex& vertex = meshData.vertices[i];
		switch (i)
		{
		case 0:
		{
			vertex.position = Vec4(static_cast<float>(line.initPosition.x), static_cast<float>(line.initPosition.y), 0.0f, 1.0f);
			break;
		}
		case 1:
		{
			vertex.position = Vec4(static_cast<float>(line.initPosition.x), static_cast<float>(line.position.y - background_height), 0.0f, 1.0f);
			break;
		}
		case 2:
		{
			vertex.position = Vec4(static_cast<float>(line.position.x + 2), static_cast<float>(line.position.y), 0.0f, 1.0f);
			break;
		}
		case 3:
		{
			vertex.position = Vec4(static_cast<float>(line.position.x + 2), static_cast<float>(line.position.y - background_height), 0.0f, 1.0f);
			break;
		}
		}
		vertex.position.y += 4.0f;
		vertex.position.y *= -1.0f;
		if (GraphicsAPI::IsD3D12())
			vertex.position.y += static_cast<float>(m_CI.viewportHeight);

		vertex.texCoord = Vec2(-1.0f, -1.0f);
		vertex.normal = Vec4(0.0f, 0.0f, 1.0f, 0.0f);
		vertex.tangent = Vec4(1.0f, 0.0f, 0.0f, 0.0f);
		vertex.binormal = Vec4(1.0f, 0.0f, 0.0f, 0.0f);
		vertex.colour = line.backgroundColour;
	}
	meshData.indices.push_back(0 + (4 * charIndex));
	meshData.indices.push_back(1 + (4 * charIndex));
	meshData.indices.push_back(2 + (4 * charIndex));
	meshData.indices.push_back(2 + (4 * charIndex));
	meshData.indices.push_back(1 + (4 * charIndex));
	meshData.indices.push_back(3 + (4 * charIndex));

	if (!update)
	{
		Model::CreateInfo textModelCI;
		textModelCI.debugName = "GEAR_CORE_FontRenderer: " + line.text;
		textModelCI.device = m_CI.device;
		textModelCI.pMesh = gear::CreateRef<Mesh>(&textMeshCI);
		textModelCI.renderPipelineName = "Font";
		line.model = gear::CreateRef<Model>(&textModelCI);
	}
	else
	{
		line.model->m_CI.pMesh = gear::CreateRef<Mesh>(&textMeshCI);
	}
}