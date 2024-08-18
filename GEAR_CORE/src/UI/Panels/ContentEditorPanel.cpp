#include "gear_core_common.h"
#include "UI/Panels/ContentEditorPanel.h"
#include "UI/Panels/ViewportPanel.h"

#include "UI/UIContext.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/ComponentUI/ModelComponentUI.h"
#include "UI/ComponentUI/MaterialComponentUI.h"
#include "UI/ComponentUI/TextureComponentUI.h"

#include "Asset/AssetFile.h"
#include "Asset/AssetManager.h"
#include "Project/Project.h"
#include "Graphics/Rendering/Renderer.h"
#include "Core/FileDialog.h"

#include "Objects/Material.h"

#include <fstream>

using namespace gear;
using namespace ui;
using namespace panels;
using namespace componentui;

using namespace core;
using namespace objects;

using namespace miru;
using namespace base;

ContentEditorPanel::ContentEditorPanel(CreateInfo* pCreateInfo)
{
	m_Type = Type::CONTENT_EDITOR;
	m_CI = *pCreateInfo;
}

ContentEditorPanel::~ContentEditorPanel()
{
	texture = nullptr;
}

void ContentEditorPanel::Draw()
{
	std::string id = UIContext::GetUIContext()->GetUniqueIDString("Content Editor", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		open = ImGui::Button("Open");
		if (open)
		{
			m_CI.currentFilepathFull = core::FileDialog_Open("", "");
		}
		open = std::filesystem::exists(m_CI.currentFilepathFull);
		ImGui::SameLine();
		close = ImGui::Button("Close");
		if (close)
			open = close;
		
		ImGui::SameLine();
		ImGui::Text(m_CI.currentFilepathFull.string().c_str());

		if (m_CI.filepathExt.empty())
			m_CI.filepathExt = m_CI.currentFilepathFull.extension().string();

		ContentType contentType = GetContextTypeFromExtension(m_CI.filepathExt);
		if (contentType == ContentType::TEXT)
		{
			if (open)
			{
				std::fstream stream;
				if (!stream.is_open())
					stream.open(m_CI.currentFilepathFull.string(), std::ios::ate | std::ios::in);

				if (stream.is_open() && read)
				{
					size_t fileSize = static_cast<size_t>(stream.tellg());
					stream.seekg(0, std::fstream::beg);
					std::string line;
					while (!stream.eof())
					{
						std::getline(stream, line);
						output.append(line + "\n");
					}
					stream.close();
					read = false;
				}
				
				if (close)
				{
					output.clear();
					open = close = false;
					read = true;
				}
			}

			if (!output.empty())
				ImGui::TextUnformatted(output.c_str());
		}

		if (contentType == ContentType::IMAGE && open)
		{
			Ref<asset::AssetManager> assetManager = project::Project::GetAssetManager();
			Ref<asset::ImageAssetDataBuffer> asset = assetManager->GetAsset<asset::ImageAssetDataBuffer>(m_CI.handle);

			if (!texture && asset)
			{
				UIContext* uiContext = UIContext::GetUIContext();

				graphics::Texture::CreateInfo textureCI;
				textureCI.debugName = "TestTexture";
				textureCI.device = uiContext->GetDevice();
				textureCI.imageData = asset->Data;
				textureCI.width = asset->width;
				textureCI.height = asset->height;
				textureCI.depth = asset->depth;
				textureCI.mipLevels = graphics::Texture::MaxMipLevel;
				textureCI.arrayLayers = 1;
				textureCI.type = miru::base::Image::Type::TYPE_2D;
				textureCI.format = asset->format;
				textureCI.samples = miru::base::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
				textureCI.usage = miru::base::Image::UsageBit(0);
				textureCI.generateMipMaps = true;
				textureCI.gammaSpace = graphics::GammaSpace::SRGB;
				texture = CreateRef<graphics::Texture>(&textureCI);

				for (const auto& panel : uiContext->GetEditorPanelsByType<ViewportPanel>())
				{
					if (panel)
					{
						panel->GetRenderer()->SubmitTexturesForUpload({ texture });
						break;
					}
				}
			}
			DrawTextureComponentUI(texture);
		}

#if 0
		if (contentType == ContentType::GEAR_ASSET)
		{
			if (open)
			{
				if (read)
				{
					AssetFile assetFile = AssetFile(m_CI.currentFilepathFull.string());
					if (assetFile.Contains(AssetFile::Type::MATERIAL))
					{
						Material::CreateInfo materialCI = { m_CI.currentFilepathFull.string(), UIContext::GetUIContext()->GetDevice(), {}, {}};
						material = CreateRef<Material>(&materialCI);

						material->LoadFromAssetFile(assetFile);
						read = false;
					}
				}

				if (close)
				{
					material = nullptr;
					open = close = false;
					read = true;
				}
			}
			
			if(material)
				DrawMaterialUI(material, UIContext::GetUIContext(), false);
		}
			
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".hdr");
			if (payload)
			{
				std::string filepath = (char*)payload->Data;
				if (std::filesystem::exist(filepath))
				{
					m_CI.currentFilepath = filepath;
				}
			}
		}
#endif
	}
	ImGui::End();

	if (!m_Open)
		this->~ContentEditorPanel();
}

ContentEditorPanel::ContentType ContentEditorPanel::GetContextTypeFromExtension(const std::string& fileExtension)
{
	ContentType contentType;

	if (fileExtension.compare(".a") == 0)
		contentType = ContentType::LIBRARY_STATIC;
	else if (fileExtension.compare(".aif") == 0)
		contentType = ContentType::AUDIO;
	else if (fileExtension.compare(".avi") == 0)
		contentType = ContentType::VIDEO;
	else if (fileExtension.compare(".bak") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".bat") == 0)
		contentType = ContentType::SCRIPT;
	else if (fileExtension.compare(".bin") == 0)
		contentType = ContentType::BINARY;
	else if (fileExtension.compare(".c") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".cmake") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".cpp") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".cs") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".cso") == 0)
		contentType = ContentType::BINARY;
	else if (fileExtension.compare(".cxx") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".dll") == 0)
		contentType = ContentType::LIBRARY_DYNAMIC;
	else if (fileExtension.compare(".dmp") == 0)
		contentType = ContentType::BINARY;
	else if (fileExtension.compare(".exe") == 0)
		contentType = ContentType::BINARY;
	else if (fileExtension.compare(".exr") == 0)
		contentType = ContentType::IMAGE;
	else if (fileExtension.compare(".exp") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".fbx") == 0)
		contentType = ContentType::MODEL;
	else if (fileExtension.compare(".filters") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".h") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".hdr") == 0)
		contentType = ContentType::IMAGE;
	else if (fileExtension.compare(".hlsl") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".hpp") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".html") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".icf") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".ico") == 0)
		contentType = ContentType::IMAGE;
	else if (fileExtension.compare(".idb") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".ilk") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".ini") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".inl") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".iobj") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".ipdb") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".iso") == 0)
		contentType = ContentType::BINARY;
	else if (fileExtension.compare(".ixx") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".java") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".jpg") == 0)
		contentType = ContentType::IMAGE;
	else if (fileExtension.compare(".json") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".lib") == 0)
		contentType = ContentType::LIBRARY_STATIC;
	else if (fileExtension.compare(".lnk") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".log") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".md") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".midi") == 0)
		contentType = ContentType::AUDIO;
	else if (fileExtension.compare(".mp3") == 0)
		contentType = ContentType::AUDIO;
	else if (fileExtension.compare(".mp4") == 0)
		contentType = ContentType::VIDEO;
	else if (fileExtension.compare(".o") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".obj") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".ogg") == 0)
		contentType = ContentType::IMAGE;
	else if (fileExtension.compare(".pch") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".pdb") == 0)
		contentType = ContentType::LIBRARY_SYMBOLS;
	else if (fileExtension.compare(".pdf") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".png") == 0)
		contentType = ContentType::IMAGE;
	else if (fileExtension.compare(".psd") == 0)
		contentType = ContentType::IMAGE;
	else if (fileExtension.compare(".py") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".rar") == 0)
		contentType = ContentType::COMPRESSED;
	else if (fileExtension.compare(".sh") == 0)
		contentType = ContentType::SCRIPT;
	else if (fileExtension.compare(".sln") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".spv") == 0)
		contentType = ContentType::BINARY;
	else if (fileExtension.compare(".svg") == 0)
		contentType = ContentType::IMAGE;
	else if (fileExtension.compare(".tga") == 0)
		contentType = ContentType::IMAGE;
	else if (fileExtension.compare(".ttf") == 0)
		contentType = ContentType::FONT;
	else if (fileExtension.compare(".txt") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".user") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".vcxproj") == 0)
		contentType = ContentType::UNKNOWN;
	else if (fileExtension.compare(".wav") == 0)
		contentType = ContentType::AUDIO;
	else if (fileExtension.compare(".wma") == 0)
		contentType = ContentType::AUDIO;
	else if (fileExtension.compare(".wmv") == 0)
		contentType = ContentType::VIDEO;
	else if (fileExtension.compare(".xml") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".yml") == 0)
		contentType = ContentType::TEXT;
	else if (fileExtension.compare(".zip") == 0)
		contentType = ContentType::COMPRESSED;
	else
		contentType = ContentType::UNKNOWN;

	//Check against GEAR custom files
	if (contentType == ContentType::UNKNOWN)
	{
		if (fileExtension.compare(".gsf") == 0)
			contentType = ContentType::GEAR_SCENE;
		else if (fileExtension.compare(".gaf") == 0)
			contentType = ContentType::GEAR_ASSET;
		else if (fileExtension.compare(".grpf") == 0)
			contentType = ContentType::GEAR_RENDER_PIPELINE;
		else
			contentType = ContentType::UNKNOWN;
	}

	return contentType;
}
