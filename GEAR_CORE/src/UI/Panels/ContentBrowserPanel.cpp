#include "gear_core_common.h"
#include "UI/Panels/ContentBrowserPanel.h"
#include "UI/Panels/ContentEditorPanel.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/UIContext.h"

#include "Core/FileDialog.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"

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

	{
		CommandPool::CreateInfo cmdPoolCI = { "ContentBrowser", UIContext::GetUIContext()->GetContext(), CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT, CommandPool::QueueType::GRAPHICS };
		CommandPoolRef cmdPool = CommandPool::Create(&cmdPoolCI);

		CommandBuffer::CreateInfo cmdBufferCI = { "ContentBrowser", cmdPool, CommandBuffer::Level::PRIMARY, 1 };
		CommandBufferRef cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

		Fence::CreateInfo fenceCI = { "ContentBrowser", UIContext::GetUIContext()->GetDevice(), false, UINT64_MAX };
		FenceRef fence = Fence::Create(&fenceCI);

		cmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		std::vector<BarrierRef> barriers;
		Barrier::CreateInfo barrierCI;
		barrierCI.type = Barrier::Type::IMAGE;
		barrierCI.srcAccess = Barrier::AccessBit::NONE_BIT;
		barrierCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		barrierCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		barrierCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		barrierCI.image = m_FolderTexture->GetImage();
		barrierCI.oldLayout = Image::Layout::UNKNOWN;
		barrierCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		barrierCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		barriers.emplace_back(Barrier::Create(&barrierCI));
		barrierCI.image = m_FileTexture->GetImage();
		barriers.emplace_back(Barrier::Create(&barrierCI));
		for (auto& fileExt : m_FileExtTextureFilepath)
		{
			barrierCI.image = fileExt.texture->GetImage();
			barriers.emplace_back(Barrier::Create(&barrierCI));
		}
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, barriers);
		barriers.clear();
		
		cmdBuffer->CopyBufferToImage(0, m_FolderTexture->GetCPUBuffer(), m_FolderTexture->GetImage(), Image::Layout::TRANSFER_DST_OPTIMAL, { { 0, 0, 0, { Image::AspectBit::COLOUR_BIT, 0, 0, 1 }, { 0, 0, 0 }, { m_FolderTexture->GetWidth(), m_FolderTexture->GetHeight(), m_FolderTexture->GetDepth() }} });
		cmdBuffer->CopyBufferToImage(0, m_FileTexture->GetCPUBuffer(), m_FileTexture->GetImage(), Image::Layout::TRANSFER_DST_OPTIMAL, { { 0, 0, 0, { Image::AspectBit::COLOUR_BIT, 0, 0, 1 }, { 0, 0, 0 }, { m_FileTexture->GetWidth(), m_FileTexture->GetHeight(), m_FileTexture->GetDepth() }} });
		for (auto& fileExt : m_FileExtTextureFilepath)
			cmdBuffer->CopyBufferToImage(0, fileExt.texture->GetCPUBuffer(), fileExt.texture->GetImage(), Image::Layout::TRANSFER_DST_OPTIMAL, { { 0, 0, 0, { Image::AspectBit::COLOUR_BIT, 0, 0, 1 }, { 0, 0, 0 }, { fileExt.texture->GetWidth(), fileExt.texture->GetHeight(), fileExt.texture->GetDepth() }} });

		barrierCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		barrierCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		barrierCI.image = m_FolderTexture->GetImage();
		barrierCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		barrierCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		barriers.emplace_back(Barrier::Create(&barrierCI));
		barrierCI.image = m_FileTexture->GetImage();
		barriers.emplace_back(Barrier::Create(&barrierCI));
		for (auto& fileExt : m_FileExtTextureFilepath)
		{
			barrierCI.image = fileExt.texture->GetImage();
			barriers.emplace_back(Barrier::Create(&barrierCI));
		}
		cmdBuffer->PipelineBarrier(0, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, DependencyBit::NONE_BIT, barriers);
		barriers.clear();

		cmdBuffer->End(0);
		CommandBuffer::SubmitInfo si = { {0}, {}, {}, {}, {}, {} };
		cmdBuffer->Submit({ si }, fence);
		fence->Wait();
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
							UIContext::GetUIContext()->GetEditorPanels().emplace_back(CreateRef<ContentEditorPanel>(&contentEditorCI));
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
