#include "gearbox_common.h"
#include "ContentBrowserPanel.h"
#include "ContentEditorPanel.h"
#include "ComponentUI/ComponentUI.h"

using namespace gearbox;
using namespace panels;
using namespace componentui;

using namespace gear;
using namespace graphics;

using namespace miru::crossplatform;

ContentBrowserPanel::ContentBrowserPanel(CreateInfo* pCreateInfo)
{
	m_Type = Type::CONTENT_BROWSER;
	m_CI = *pCreateInfo;

	gear::graphics::Texture::CreateInfo tCI;
	tCI.debugName = "GEARBOX: FolderTexture";
	tCI.device = m_CI.uiContext->GetCreateInfo().window->GetDevice();
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
		std::string filepath = "icons/" + fileExt.filename;
		tCI.file.filepaths = { filepath };
		fileExt.texture = CreateRef<Texture>(&tCI);
	}

	{
		CommandPool::CreateInfo cmdPoolCI = { "ContentBrowser", m_CI.uiContext->GetCreateInfo().window->GetContext(), CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT, CommandPool::QueueType::GRAPHICS };
		Ref<CommandPool> cmdPool = CommandPool::Create(&cmdPoolCI);

		CommandBuffer::CreateInfo cmdBufferCI = { "ContentBrowser", cmdPool, CommandBuffer::Level::PRIMARY, 1, false };
		Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

		Fence::CreateInfo fenceCI = { "ContentBrowser", m_CI.uiContext->GetCreateInfo().window->GetDevice(), false, UINT64_MAX };
		Ref<Fence> fence = Fence::Create(&fenceCI);

		cmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		std::vector<Ref<Barrier>> barriers;
		m_FolderTexture->TransitionSubResources(barriers, { { Barrier::AccessBit::NONE_BIT, Barrier::AccessBit::TRANSFER_WRITE_BIT, Image::Layout::UNKNOWN, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
		m_FileTexture->TransitionSubResources(barriers, { { Barrier::AccessBit::NONE_BIT, Barrier::AccessBit::TRANSFER_WRITE_BIT, Image::Layout::UNKNOWN, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
		for (auto& fileExt : m_FileExtTextureFilepath)
			fileExt.texture->TransitionSubResources(barriers, { { Barrier::AccessBit::NONE_BIT, Barrier::AccessBit::TRANSFER_WRITE_BIT, Image::Layout::UNKNOWN, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, barriers);
		barriers.clear();
		
		m_FolderTexture->Upload(cmdBuffer);
		m_FileTexture->Upload(cmdBuffer);
		for (auto& fileExt : m_FileExtTextureFilepath)
			fileExt.texture->Upload(cmdBuffer);

		m_FolderTexture->TransitionSubResources(barriers, { { Barrier::AccessBit::TRANSFER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT, Image::Layout::TRANSFER_DST_OPTIMAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
		m_FileTexture->TransitionSubResources(barriers, { { Barrier::AccessBit::TRANSFER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT, Image::Layout::TRANSFER_DST_OPTIMAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
		for (auto& fileExt : m_FileExtTextureFilepath)
			fileExt.texture->TransitionSubResources(barriers, { { Barrier::AccessBit::TRANSFER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT, Image::Layout::TRANSFER_DST_OPTIMAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		barriers.clear();

		cmdBuffer->End(0);
		cmdBuffer->Submit({ 0 }, {}, {}, {}, fence);
		fence->Wait();
	}

	m_FolderTextureID = GetTextureID(m_FolderTexture->GetTextureImageView(), m_CI.uiContext, false, 0);
	m_FileTextureID = GetTextureID(m_FileTexture->GetTextureImageView(), m_CI.uiContext, false, 0);
	for (auto& fileExt : m_FileExtTextureFilepath)
		fileExt.id= GetTextureID(fileExt.texture->GetTextureImageView(), m_CI.uiContext, false, 0);
	
}

ContentBrowserPanel::~ContentBrowserPanel()
{
	m_CI.uiContext->GetCreateInfo().window->GetContext()->DeviceWaitIdle();
	for (auto& fileExt : m_FileExtTextureFilepath)
	{
		fileExt.texture = nullptr; //Call the destructor.
	}
}

void ContentBrowserPanel::Draw()
{
	if (ImGui::Begin("Content Browser", &m_Open))
	{
		
		if (ImGui::Button("/\\"))
		{
			m_CI.currentPath = m_CI.currentPath.parent_path();
		}
		static float iconSize = 128.0f;
		//DrawFloat("Icon Size", iconSize, 32.0f, 256.0f, 100.0f, 1.0f);

		std::string currentPathStr = m_CI.currentPath.generic_string();
		DrawInputText("Current Path", currentPathStr);
		m_CI.currentPath = currentPathStr;

		//Order the directory entries to be folder first
		std::filesystem::directory_iterator dirIt(m_CI.currentPath);
		std::vector<std::filesystem::directory_entry> directories;
		std::copy(dirIt, std::filesystem::directory_iterator(), std::back_inserter(directories));
		std::sort(directories.begin(), directories.end(), 
			[](const std::filesystem::directory_entry& de1, const std::filesystem::directory_entry& de2) 
			{
				bool a = de1.is_directory(); 
				bool b = de2.is_directory(); 
				return a == b ? false : a && !b ? true : !a && b ? false : false;
			});

		//Calculate number of coloumns
		uint32_t iconsPerRow = std::max(uint32_t(ImGui::GetContentRegionAvailWidth() / iconSize), uint32_t(1));
		ImGui::Columns(static_cast<int>(iconsPerRow), 0, false);
		int i = 0;

		for (auto& directory : directories)
		{
			const std::filesystem::path& path = directory.path().lexically_relative(m_CI.currentPath);
			const std::string& pathStr = path.generic_u8string();
			const std::string& pathExtStr = path.extension().generic_string();

			//ImGui::SetColumnWidth(i++, iconSize + 16);
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
			if (imageID != 0)
				ImGui::ImageButton(imageID, ImVec2(iconSize, iconSize));
			else
				ImGui::Button(pathStr.c_str(), ImVec2(iconSize, iconSize));

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
					contentEditorCI.uiContext = m_CI.uiContext;
					contentEditorCI.currentFilepath = path;
					contentEditorCI.currentFilepathFull = directory.path();
					contentEditorCI.filepathExt = pathExtStr;
					m_CI.uiContext->GetEditorPanels().emplace_back(CreateRef<ContentEditorPanel>(&contentEditorCI));
				}
			}

			//Icon name wrapped
			ImGui::TextWrapped(pathStr.c_str());
			ImGui::NextColumn();
			ImGui::PopID();
		}

		ImGui::Columns(1);
	}
	ImGui::End();
}
