#include "gear_core_common.h"
#include "UI/Panels/ContentBrowserPanel.h"
#include "UI/Panels/ContentEditorPanel.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/UIContext.h"

#include "Core/FileDialog.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"
#include "Graphics/Rendering/Renderer.h"

using namespace gear;
using namespace graphics;
using namespace ui;
using namespace panels;
using namespace componentui;

using namespace miru::base;

ContentBrowserPanel::ContentBrowserPanel(CreateInfo* pCreateInfo)
{
	m_Type = Type::CONTENT_BROWSER;
	m_CI = *pCreateInfo;

	Texture::CreateInfo tCI;
	tCI.debugName = "GEARBOX: FolderTexture";
	tCI.device = UIContext::GetUIContext()->GetDevice();
	tCI.dataType = Texture::DataType::FILE;
	tCI.file.filepaths = { m_FolderTextureFilepath };
	tCI.mipLevels = 1;
	tCI.arrayLayers = 1;
	tCI.type = Image::Type::TYPE_2D;
	tCI.format = Image::Format::R8G8B8A8_UNORM;
	tCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	tCI.usage = Image::UsageBit::SAMPLED_BIT;
	tCI.generateMipMaps = false;
	tCI.gammaSpace = GammaSpace::SRGB;
	m_FolderTexture = CreateRef<Texture>(&tCI);
	tCI.debugName = "GEARBOX: FileTexture";
	tCI.file.filepaths = { m_FileTextureFilepath };
	m_FileTexture = CreateRef<Texture>(&tCI);
	for (auto& fileExt : m_FileExtTextureFilepath)
	{
		tCI.debugName = "GEARBOX: FileTexture: " + arc::ToUpper(fileExt.extension);
		std::string filepath = fileExt.filename;
		tCI.file.filepaths = { filepath };
		fileExt.texture = CreateRef<Texture>(&tCI);
	}

	m_FolderTextureID = GetTextureID(m_FolderTexture->GetImageView(), UIContext::GetUIContext(), false);
	m_FileTextureID = GetTextureID(m_FileTexture->GetImageView(), UIContext::GetUIContext(), false);
	for (auto& fileExt : m_FileExtTextureFilepath)
		fileExt.id= GetTextureID(fileExt.texture->GetImageView(), UIContext::GetUIContext(), false);
	
}

ContentBrowserPanel::~ContentBrowserPanel()
{
	UIContext::GetUIContext()->GetWindow()->GetContext()->DeviceWaitIdle();
	for (auto& fileExt : m_FileExtTextureFilepath)
	{
		fileExt.texture = nullptr; //Call the destructor.
	}
}

void ContentBrowserPanel::Draw()
{
	UIContext* uiContext = UIContext::GetUIContext();
	if (!uiContext)
		return;

	//Submit Folder and File Texture to be uploaded
	if (m_UploadFolderAndFileTextures)
	{
		for (auto& panel : uiContext->GetEditorPanelsByType<ViewportPanel>())
		{
			if (panel)
			{
				std::vector<Ref<Texture>> textures;
				textures.reserve(m_FileExtTextureFilepath.size() + 2);
				textures.push_back(m_FolderTexture);
				textures.push_back(m_FileTexture);
				for (auto& fileExt : m_FileExtTextureFilepath)
					textures.push_back(fileExt.texture);

				panel->GetCreateInfo().renderer->SubmitTexturesForUpload(textures);
				m_UploadFolderAndFileTextures = false;
				break;
			}
		}
	}

	std::string id = UIContext::GetUIContext()->GetUniqueIDString<ContentBrowserPanel>("Content Browser", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		if (ImGui::ImageButton(m_FolderTextureID, { ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() }))
		{
			m_CI.currentPath = core::FolderDialog_Browse();
		}
		ImGui::SameLine();

		if (ImGui::Button("/\\"))
		{
			m_CI.currentPath = m_CI.currentPath.parent_path();
		}
		ImGui::SameLine();

		std::string currentPathStr = m_CI.currentPath.string();
		DrawInputText("Current Path", currentPathStr);

		if (std::filesystem::exists(std::filesystem::path(currentPathStr)))
		{
			m_CI.currentPath = currentPathStr;

			//Order the directory entries to be folder first
			std::filesystem::directory_iterator directory_it(m_CI.currentPath);
			std::vector<std::filesystem::directory_entry> directories;
			std::copy(directory_it, std::filesystem::directory_iterator(), std::back_inserter(directories));
			std::sort(directories.begin(), directories.end(), 
				[](const std::filesystem::directory_entry& de1, const std::filesystem::directory_entry& de2) 
				{
					bool a = de1.is_directory(); 
					bool b = de2.is_directory(); 
					return a == b ? false : a && !b ? true : !a && b ? false : false;
				});

			//Calculate number of coloumns
			static float iconSize = 128.0f;
			uint32_t iconsPerRow = std::max(uint32_t(ImGui::GetContentRegionAvail().x / iconSize), uint32_t(1));
			if (ImGui::BeginTable("##ContentBrowserPanel", static_cast<int>(iconsPerRow)))
			{
				uint32_t i = 0;
				for (auto& directory : directories)
				{
					if(i % iconsPerRow == 0)
						ImGui::TableNextRow();
					ImGui::TableNextColumn();
					i++;

					const std::filesystem::path& path = directory.path().lexically_relative(m_CI.currentPath);
					const std::string& pathStr = path.string();
					const std::string& pathExtStr = path.extension().string();

					ImGui::PushID(pathStr.c_str());

					//Get icon texture
					ImTextureID imageID = 0;
					for (auto& fileExt : m_FileExtTextureFilepath)
					{
						bool found = fileExt.extension.compare(pathExtStr) == 0;
						if (found)
						{
							imageID = fileExt.id;
							break;
						}
						else
						{
							continue;
						}
					}
					if (m_FolderTextureID && m_FileTextureID && imageID == 0)
					{
						imageID = directory.is_directory() ? m_FolderTextureID : m_FileTextureID;
					}

					//Draw icon
					ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
					ImGui::ImageButton(imageID, ImVec2(iconSize, iconSize));

					//DragDrop operations
					if (ImGui::BeginDragDropSource())
					{
						std::string fullpath = directory.path().string();
						ImGui::SetDragDropPayload(pathExtStr.c_str(), (const void*)(fullpath.c_str()), fullpath.size() + 1);
						ImGui::EndDragDropSource();
					}
					ImGui::PopStyleColor();

					//Double click icon
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						if (directory.is_directory())
						{
							m_CI.currentPath /= path;
						}
						else
						{
							ContentEditorPanel::CreateInfo contentEditorCI;
							contentEditorCI.currentFilepathFull = directory.path();
							contentEditorCI.filepathExt = pathExtStr;
							uiContext->GetEditorPanels().emplace_back(CreateRef<ContentEditorPanel>(&contentEditorCI));
						}
					}

					//Icon name wrapped
					ImGui::TextWrapped("%s", path.u8string().c_str());
					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}

	}
	ImGui::End();
}
