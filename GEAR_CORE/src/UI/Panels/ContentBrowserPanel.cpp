#include "gear_core_common.h"
#include "UI/Panels/ContentBrowserPanel.h"
#include "UI/Panels/ContentEditorPanel.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/UIContext.h"

#include "Asset/AssetFile.h"
#include "Asset/EditorAssetManager.h"
#include "Project/Project.h"
#include "Core/FileDialog.h"

#include "Graphics/Texture.h"
#include "Graphics/Window.h"
#include "Graphics/Rendering/Renderer.h"

#include "yaml-cpp/yaml.h"

#include "imgui/misc/cpp/imgui_stdlib.h"
#include "GLFW/include/GLFW/glfw3.h"

using namespace gear;
using namespace graphics;
using namespace ui;
using namespace panels;
using namespace componentui;
using namespace asset;
using namespace project;

using namespace miru::base;

static float iconSize = 128.0f;

ContentBrowserPanel::ContentBrowserPanel(CreateInfo* pCreateInfo)
{
	m_Type = Type::CONTENT_BROWSER;
	m_CI = *pCreateInfo;
	
	m_CurrentPath = m_CI.AssetFolderPath;
	const std::filesystem::path& sourceDir = UIContext::GetUIContext()->GetSourceDirectory();

	const Ref<EditorAssetManager>& editorAssetManager = UIContext::GetUIContext()->GetEditorAssetManager();
	Ref<ImageAssetDataBuffer> imageData = nullptr;

	Texture::CreateInfo tCI;
	tCI.debugName = "GEARBOX: FolderTexture";
	tCI.device = UIContext::GetUIContext()->GetDevice();
	imageData = editorAssetManager->Import<ImageAssetDataBuffer>(Asset::Type::EXTERNAL_FILE, sourceDir / m_FolderTextureFilepath);
	tCI.imageData = imageData->Data;
	tCI.width = imageData->width;
	tCI.height = imageData->height;
	tCI.depth = imageData->depth;
	tCI.mipLevels = 1;
	tCI.arrayLayers = 1;
	tCI.type = Image::Type::TYPE_2D;
	tCI.format = imageData->format;
	tCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	tCI.usage = Image::UsageBit::SAMPLED_BIT;
	tCI.generateMipMaps = false;
	tCI.gammaSpace = GammaSpace::SRGB;
	m_FolderTexture = CreateRef<Texture>(&tCI);

	tCI.debugName = "GEARBOX: FileTexture";
	imageData = editorAssetManager->Import<ImageAssetDataBuffer>(Asset::Type::EXTERNAL_FILE, sourceDir / m_FileTextureFilepath);
	tCI.imageData = imageData->Data;
	tCI.width = imageData->width;
	tCI.height = imageData->height;
	tCI.depth = imageData->depth;
	tCI.format = imageData->format;
	m_FileTexture = CreateRef<Texture>(&tCI);
	for (auto& fileExt : m_FileExtTextureFilepath)
	{
		tCI.debugName = "GEARBOX: FileTexture: " + arc::ToUpper(fileExt.extension.generic_string());
		imageData = editorAssetManager->Import<ImageAssetDataBuffer>(Asset::Type::EXTERNAL_FILE, sourceDir / fileExt.filename);
		tCI.imageData = imageData->Data;
		tCI.width = imageData->width;
		tCI.height = imageData->height;
		tCI.depth = imageData->depth;
		tCI.format = imageData->format;
		fileExt.texture = CreateRef<Texture>(&tCI);
	}
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

				panel->GetRenderer()->SubmitTexturesForUpload(textures);
				m_UploadFolderAndFileTextures = false;
				break;
			}
		}
	}

	std::string id = UIContext::GetUIContext()->GetUniqueIDString<ContentBrowserPanel>("Content Browser", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		if (DrawTopBar())
		{
			DrawIcons();
			DrawContextMenu();
		}
	}
	ImGui::End();
}

bool ContentBrowserPanel::DrawTopBar()
{
	ImTextureID folderTextureID = UIContext::GetUIContext()->AddTextureID(m_FolderTexture->GetImageView());
	if (ImGui::ImageButton(folderTextureID, { ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() }))
	{
		m_CurrentPath = core::FolderDialog_Browse();
	}
	ImGui::SameLine();

	if (ImGui::Button("/\\"))
	{
		m_CurrentPath = m_CurrentPath.parent_path();
	}
	ImGui::SameLine();

	std::string currentPathStr = m_CurrentPath.string();
	DrawInputText("Current Path", currentPathStr);

	if (std::filesystem::exists(std::filesystem::path(currentPathStr)))
	{
		m_CurrentPath = currentPathStr;
		return true;
	}
	else
	{
		return false;
	}
}

void ContentBrowserPanel::DrawIcons()
{
	//Order the directory entries to be folder first
	std::filesystem::directory_iterator directory_it(m_CurrentPath);
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
	uint32_t iconsPerRow = std::max(uint32_t(ImGui::GetContentRegionAvail().x / iconSize), uint32_t(1));
	if (ImGui::BeginTable("##ContentBrowserPanel", static_cast<int>(iconsPerRow)))
	{
		uint32_t i = 0;
		for (const auto& directory : directories)
		{
			if (i % iconsPerRow == 0)
				ImGui::TableNextRow();
			ImGui::TableNextColumn();
			i++;

			DrawIcon(directory);
		}

		ImGui::EndTable();
	}

	//MultiSelect
	/*static ImGuiSelectionBasicStorage selection;
	selection.UserData = (void*)&directories;
	selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self, int idx) {
		const std::vector<std::filesystem::directory_entry>& directories = *(std::vector<std::filesystem::directory_entry>*)(self->UserData);
		return (directories[idx].;
		};

	ImGuiMultiSelectIO* multiSelectIO = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_None, selection.Size, (int)directories.size());
	selection.ApplyRequests(multiSelectIO);
	for (size_t i = 0; i < directories.size(); i++)
	{
		bool item_is_selected = selection.Contains(directories[i].ID);
		ImGui::SetNextItemSelectionUserData(i);
		ImGui::Selectable(label, item_is_selected);
	}
	multiSelectIO = ImGui::EndMultiSelect();
	selection.ApplyRequests(multiSelectIO);*/


	//Draw any pop ups
	if (m_PopupWindowFunction)
		m_PopupWindowFunction();
}

void ContentBrowserPanel::DrawIcon(const std::filesystem::directory_entry& directory)
{
	const std::filesystem::path& path = directory.path().lexically_relative(m_CurrentPath);
	const std::string& pathStr = path.string();
	const std::string& pathExtStr = path.extension().string();

	//File State
	FileState fileState = FileState::NORMAL;
	auto modifyingFile_it = m_ModifyingFiles.find(directory.path());
	if (modifyingFile_it != m_ModifyingFiles.end())
		fileState = modifyingFile_it->second;

	ImGui::PushID(pathStr.c_str());

	//Get icon texture
	ImTextureID imageID = 0;
	for (auto& fileExt : m_FileExtTextureFilepath)
	{
		bool found = fileExt.extension.compare(pathExtStr) == 0;
		if (found)
		{
			imageID = UIContext::GetUIContext()->AddTextureID(fileExt.texture->GetImageView());
			break;
		}
		else
		{
			continue;
		}
	}
	ImTextureID folderTextureID = UIContext::GetUIContext()->AddTextureID(m_FolderTexture->GetImageView());
	ImTextureID fileTextureID = UIContext::GetUIContext()->AddTextureID(m_FileTexture->GetImageView());
	if (folderTextureID && fileTextureID && imageID == 0)
	{
		imageID = directory.is_directory() ? folderTextureID : fileTextureID;
	}

	//Draw icon
	ImVec4 iconColourMultiplier = fileState == FileState::CUT ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::ImageButton(path.generic_string().c_str(), imageID, ImVec2(iconSize, iconSize), 
		ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), iconColourMultiplier);
	ImGui::PopStyleColor();

	//Hovered icon
	bool hovered = ImGui::IsItemHovered();

	//Single click icon
	bool selected = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

	//Double click icon
	if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		if (directory.is_directory())
		{
			m_CurrentPath /= path;
		}
		else
		{
			ContentEditorPanel::CreateInfo contentEditorCI;
			contentEditorCI.currentFilepathFull = directory.path();
			UIContext::GetUIContext()->AddEditorPanel(CreateRef<ContentEditorPanel>(&contentEditorCI));
		}
	}

	//Right Click Menu
	DrawIconContextMenu(directory, hovered);

	//DragDrop operations
	if (ImGui::BeginDragDropSource())
	{
		std::string fullpath = directory.path().string();
		ImGui::SetDragDropPayload(pathExtStr.c_str(), (const void*)(fullpath.c_str()), fullpath.size() + 1);
		ImGui::EndDragDropSource();
	}

	//Icon name wrapped
	if (fileState == FileState::RENAME)
	{
		static std::string filepath = path.generic_string();
		if (ImGui::InputText("##FileRename", &filepath, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::filesystem::rename(path, std::filesystem::path(filepath));
			modifyingFile_it = m_ModifyingFiles.erase(modifyingFile_it);
		}
		//ImGui::SetKeyboardFocusHere();
	}
	else 
	{
		ImGui::TextWrapped("%s", path.u8string().c_str());
	}
	
	ImGui::PopID();
}

void ContentBrowserPanel::DrawIconContextMenu(const std::filesystem::directory_entry& directory, bool hovered)
{

	bool op_cut = UIContext::ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_X });
	bool op_copy = UIContext::ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_C });
	bool op_delete = UIContext::ShortcutPressed({ GLFW_KEY_DELETE });
	bool op_rename = UIContext::ShortcutPressed({ GLFW_KEY_F2 });

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Cut", "Crtl+X"))
		{
			op_cut = true;
		}
		if (ImGui::MenuItem("Copy", "Crtl+C"))
		{
			op_copy = true;
		}
		if (ImGui::MenuItem("Delete", "Delete"))
		{
			op_delete = true;
		}
		if (ImGui::MenuItem("Rename", "F2"))
		{
			op_rename = true;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Import Asset"))
		{
			m_PopupWindowFunction = std::bind(&ContentBrowserPanel::ImportAssetDialog, this);
			m_PopupWindowFilepath = directory.path().lexically_relative(m_CI.AssetFolderPath);
		}
		ImGui::EndPopup();
	}

	if (hovered)
	{
		if (op_cut)
		{
			m_ModifyingFiles[directory.path()] = FileState::CUT;
		}
		if (op_copy)
		{
			m_ModifyingFiles[directory.path()] = FileState::COPY;
		}
		if (op_delete)
		{
			std::error_code ec;
			std::filesystem::remove(directory.path(), ec);
			if (ec)
			{
				GEAR_INFO(ErrorCode::UI | ErrorCode::NOT_SUPPORTED, "Failed to delete path: %s.", directory.path().generic_string().c_str())
			}
		}
		if (op_rename)
		{
			m_ModifyingFiles[directory.path()] = FileState::RENAME;
		}
	}
}

void ContentBrowserPanel::DrawContextMenu()
{
	bool op_paste = UIContext::ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_V });

	if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
	{
		if (!m_ModifyingFiles.empty() && ImGui::MenuItem("Paste", "Crtl+V"))
		{
			op_paste = true;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Texture"))
		{
			m_PopupWindowFunction = std::bind(&ContentBrowserPanel::CreateFileDialog, this);
			m_PopupWindowFilepath = "T_Texture.gaf";
			m_PopupWindowCloseCallback = [](const std::filesystem::path& filepath) {
				AssetMetadata metadata;
				metadata.type = Asset::Type::TEXTURE;
				metadata.filepath = filepath;
				AssetFile(metadata).Save(YAML::Emitter()); };

		}
		if (ImGui::MenuItem("Create Material"))
		{
			m_PopupWindowFunction = std::bind(&ContentBrowserPanel::CreateFileDialog, this);
			m_PopupWindowFilepath = "M_Material.gaf";
			m_PopupWindowCloseCallback = [](const std::filesystem::path& filepath) {
				AssetMetadata metadata;
				metadata.type = Asset::Type::MATERIAL;
				metadata.filepath = filepath;
				AssetFile(metadata).Save(YAML::Emitter()); };
		}
		ImGui::EndPopup();
	}

	if (op_paste)
	{
		for (auto it = m_ModifyingFiles.begin(); it != m_ModifyingFiles.end(); )
		{
			auto modifyingFile = *it;
			if (std::filesystem::exists(modifyingFile.first) && (modifyingFile.second == FileState::CUT || modifyingFile.second == FileState::COPY))
			{
				const std::filesystem::path& modifyingFilepath = modifyingFile.first;
				std::filesystem::path newFilepath = m_CurrentPath / modifyingFilepath.stem();
				std::string newFilepathStr = newFilepath.generic_string();
				while (std::filesystem::exists(newFilepathStr + modifyingFilepath.extension().generic_string()) && modifyingFile.second == FileState::COPY)
				{
					newFilepathStr += "-Copy";
				}
				newFilepathStr += modifyingFilepath.extension().generic_string();
				newFilepath = newFilepathStr;

				if (modifyingFilepath != newFilepath)
				{
					std::filesystem::copy(modifyingFilepath, newFilepath);

					if (modifyingFile.second == FileState::CUT)
					{
						std::filesystem::remove(modifyingFilepath);
					}
				}

				it = m_ModifyingFiles.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}

static std::string filepathStr = "Filename";

void ContentBrowserPanel::CreateFileDialog()
{
	if (!ImGui::IsPopupOpen("Create File"))
		ImGui::OpenPopup("Create File");

	if (ImGui::BeginPopupModal("Create File"))
	{
		std::filesystem::path& filepath = m_PopupWindowFilepath;
		filepathStr = filepath.generic_string();

		ImGui::BeginTable("##CreateFile", 3);
		ImGui::TableNextColumn();
		ImGui::Text("Filepath");

		ImGui::TableNextColumn();
		ImGui::Text(m_CI.AssetFolderPath.generic_string().c_str());
		
		ImGui::TableNextColumn();
		if (ImGui::InputText("##CreateFileFilename", &filepathStr))
			filepath = filepathStr;
	
		ImGui::EndTable();

		const std::filesystem::path& fullFilepath = m_CI.AssetFolderPath / filepath;
		static bool created = false;
		if (ImGui::Button("Create"))
		{
			std::ofstream stream(m_CI.AssetFolderPath / filepath);
			stream.close();

			created = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close") || created)
		{
			ImGui::CloseCurrentPopup();
			//Call the callback on Popup close.
			if (m_PopupWindowCloseCallback && std::filesystem::exists(fullFilepath))
				m_PopupWindowCloseCallback(fullFilepath);

			m_PopupWindowCloseCallback = nullptr;
			m_PopupWindowFunction = nullptr;
			m_PopupWindowFilepath = "";

		}
		ImGui::EndPopup();
	}
}

void ContentBrowserPanel::ImportAssetDialog()
{
	const std::filesystem::path importFilepath = m_PopupWindowFilepath;
	std::filesystem::path filepathExt = importFilepath.extension();
	bool externalFileType = !(filepathExt.generic_string().find(".gaf") != std::string::npos || filepathExt.generic_string().find(".gsf") != std::string::npos);
	if (!externalFileType)
		return;

	if (!ImGui::IsPopupOpen("Import Asset"))
		ImGui::OpenPopup("Import Asset");

	if (ImGui::BeginPopupModal("Import Asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Import File: %s", importFilepath.generic_string().c_str());
		static Asset::Type type = Asset::Type::EXTERNAL_FILE;
		if (ImGui::Button("Import"))
		{
			Ref<Project> project = UIContext::GetUIContext()->GetProject();
			if (project)
			{
				project->GetEditorAssetManager()->Import(type, importFilepath);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
			//Call the callback on Popup close.
			if (m_PopupWindowCloseCallback && std::filesystem::exists(importFilepath))
				m_PopupWindowCloseCallback(importFilepath);

			m_PopupWindowCloseCallback = nullptr;
			m_PopupWindowFunction = nullptr;
			m_PopupWindowFilepath = "";

		}
		ImGui::EndPopup();
	}
}