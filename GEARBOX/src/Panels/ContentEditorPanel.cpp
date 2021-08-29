#include "gearbox_common.h"
#include "ContentEditorPanel.h"
#include "ComponentUI/ComponentUI.h"

#include "ARC/src/FileSystemHelpers.h"

using namespace gearbox;
using namespace panels;
using namespace componentui;

using namespace gear;
using namespace graphics;

using namespace miru::crossplatform;

ContentEditorPanel::ContentEditorPanel(CreateInfo* pCreateInfo)
{
	m_Type = Type::CONTENT_EDITOR;
	m_CI = *pCreateInfo;
}

ContentEditorPanel::~ContentEditorPanel()
{
}

void ContentEditorPanel::Draw()
{
	std::string title = "Content Editor: " + m_CI.currentFilepath.string();
	if (ImGui::Begin(title.c_str(), &m_Open))
	{
		ContentType contentType = GetContextTypeFromExtension(m_CI.filepathExt);
		if (contentType == ContentType::TEXT)
		{
			open = ImGui::Button("Open");
			ImGui::SameLine();
			close = ImGui::Button("Close");
			if (close)
				open = close;
			ImGui::SameLine();
			ImGui::Text("File Size: %.1f kB", float(fileSize)/1000.0f);

			static std::fstream stream;
			if (open)
			{
				if (!stream.is_open())
					stream.open(m_CI.currentFilepathFull.string(), std::ios::ate | std::ios::in);

				if (stream.is_open() && read)
				{
					fileSize = static_cast<size_t>(stream.tellg());
					stream.seekg(0, std::fstream::beg);
					std::string line;
					while (!stream.eof())
					{
						std::getline(stream, line);
						output.append(line + "\n");
					}
					read = false;
				}
				
				if (close)
				{
					stream.close();
					output.clear();
					open = close = false;
					read = true;

				}
			}

			ImGui::Text(output.c_str());
		}
			
		/*if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".hdr");
			if (payload)
			{
				std::string filepath = (char*)payload->Data;
				if (arc::FileExist(filepath))
				{
					m_CI.currentFilepath = filepath;
				}
			}
		}*/
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
	return contentType;
}
