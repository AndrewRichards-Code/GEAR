#include "gear_core_common.h"
#include "UI/Panels/OutputPanel.h"
#include "UI/UIContext.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "Core/Application.h"

using namespace gear;
using namespace core;
using namespace ui;
using namespace panels;
using namespace componentui;

std::deque<OutputPanel::Entry> OutputPanel::s_OutputEntries;

OutputPanel::OutputPanel()
{
	m_Type = Type::OUTPUT;

	Ref<arc::VisualStudioDebugOutput> debugOuput = Application::GetApplication()->GetApplicationContext().GetVisualStudioDebugOutput();
	debugOuput->SetCallback(DebugMessageCallback);
	s_OutputEntries.clear();
}

OutputPanel::~OutputPanel()
{
	Ref<arc::VisualStudioDebugOutput> debugOuput = Application::GetApplication()->GetApplicationContext().GetVisualStudioDebugOutput();
	debugOuput->SetCallback(nullptr);
	s_OutputEntries.clear();
}

void OutputPanel::Draw()
{
	std::string id = UIContext::GetUIContext()->GetUniqueIDString("Output", this);
	if (ImGui::Begin(id.c_str(), &m_Open))
	{
		static bool parsed = true;
		DrawToggleButton("Raw - Parsed", parsed);

		static bool autoScroll = true;
		ImGui::SameLine();
		ImGui::Checkbox("Auto Scroll", &autoScroll);

		static bool wordWrap = true;
		if (!parsed)
		{
			ImGui::SameLine();
			ImGui::Checkbox("Word Wrap", &wordWrap);
		}

		ImGui::SameLine();
		if (ImGui::Button("Clear"))
			s_OutputEntries.clear();

		ImGui::Separator();

		bool table = false;
		if (parsed)
		{
			table = ImGui::BeginTable("##outputTable", 8,
				ImGuiTableFlags_::ImGuiTableFlags_Resizable |
				ImGuiTableFlags_::ImGuiTableFlags_Hideable |
				ImGuiTableFlags_::ImGuiTableFlags_ContextMenuInBody |
				ImGuiTableFlags_::ImGuiTableFlags_ScrollX |
				ImGuiTableFlags_::ImGuiTableFlags_ScrollY
			);
			if (table)
			{
				const int& columnCount = ImGui::TableGetColumnCount();
				const char* columnNames[8] = { "Date", "Time", "Logger", "Level", "File(Line)", "Function", "ErrorCode", "Message" };
				for (int i = 0; i < columnCount; i++)
				{
					ImGui::TableSetupColumn(columnNames[i]);
				}
				ImGui::TableSetupScrollFreeze(columnCount, 1);
				ImGui::TableNextRow(ImGuiTableRowFlags_::ImGuiTableRowFlags_Headers);
				for (int i = 0; i < columnCount; i++)
				{
					ImGui::TableNextColumn(); ImGui::Text(columnNames[i]);
				}
			}
		}
		else
		{
			ImGui::BeginChild("##output");
		}


		for (const Entry& entry : s_OutputEntries)
		{
			if (!parsed)
			{
				bool red = (entry.logLevel.compare("ERROR") == 0);
				bool yellow = (entry.logLevel.compare("WARNING") == 0);

				if (red)
					ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));
				if (yellow)
					ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.1f, 1.0f));

				if (wordWrap)
					ImGui::TextWrapped(entry.rawString.c_str());
				else
					ImGui::Text(entry.rawString.c_str());

				if (red || yellow)
					ImGui::PopStyleColor();
			}
			else
			{
				if (table)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::Text(entry.date.c_str());
					ImGui::TableNextColumn(); ImGui::Text(entry.time.c_str());
					ImGui::TableNextColumn(); ImGui::Text(entry.loggerName.c_str());
					ImGui::TableNextColumn(); ImGui::Text(entry.logLevel.c_str());
					ImGui::TableNextColumn(); ImGui::TextWrapped(entry.fileAndLine.c_str());
					ImGui::TableNextColumn(); ImGui::TextWrapped(entry.functionSignature.c_str());
					ImGui::TableNextColumn(); ImGui::Text(entry.errorCode.c_str());
					ImGui::TableNextColumn(); ImGui::TextWrapped(entry.message.c_str());
				}
			}
		}

		if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);
		
		if (table)
		{
			ImGui::EndTable();
		}
		else
		{
			ImGui::EndChild();
		}



		ImGui::End();
	}
}

void OutputPanel::DebugMessageCallback(const std::string& string)
{
	Entry entry;

	std::stringstream ss(string);
	std::vector<std::string> results;
	std::string item;
	while (std::getline(ss, item, ']'))
	{
		results.push_back(item);
	}
	for (size_t i = 0; i < 3; i++)
	{
		if (results[i].find('[') != std::string::npos)
			results[i].erase(results[i].find('['), 1);
	}
	if (results.size() > 4)
	{
		for (size_t i = 4; i < results.size(); i++)
			results[3].append(results[i]);
	}
	if (results[3].find_last_of("\n") != std::string::npos)
	{
		results[3].erase(results[3].find_last_of("\n"));
	}

	const std::string delim0 = " ";
	const std::string delim1 = ".";
	const size_t delimSize0 = delim0.size();
	{
		std::string string = results[0];
		size_t pos0 = 0;
		size_t pos1 = string.find(delim0, pos0);
		entry.date = string.substr(pos0, pos1 - pos0);

		pos0 = pos1 + delimSize0;
		pos1 = string.find(delim1, pos0);
		entry.time = string.substr(pos0, pos1 - pos0);
	}
	const std::string delim = ": ";
	const size_t delimSize = delim.size();
	{
		std::string string = results[1];
		size_t pos0 = 0;
		size_t pos1 = string.find(delim, pos0);
		entry.loggerName = string.substr(pos0, pos1 - pos0);
		
		pos0 = pos1 + delimSize;
		pos1 = string.find(delim, pos0);
		entry.logLevel = string.substr(pos0, pos1 - pos0);

		pos0 = pos1 + delimSize;
		pos1 = string.find(delim, pos0);
		entry.fileAndLine = string.substr(pos0, pos1 - pos0);

		pos0 = pos1 + delimSize;
		pos1 = string.find(delim, pos0);
		entry.functionSignature = string.substr(pos0, pos1 - pos0);
	}
	{
		std::string string = results[2];
		size_t pos0 = 0;
		size_t pos1 = string.find(delim, pos0);
		pos0 = pos1 + delimSize;
		pos1 = string.find(delim, pos0);
		entry.errorCode = string.substr(pos0, pos1 - pos0);
	}
	{
		std::string string = results[3];
		size_t pos0 = 0;
		size_t pos1 = string.find(delim, pos0);
		pos0 = pos1 + delimSize;
		pos1 = string.find(delim, pos0);
		entry.message = string.substr(pos0);

	}
	entry.rawString = string;
	
	s_OutputEntries.push_back(entry);
	if (s_OutputEntries.size() > 1024)
		s_OutputEntries.pop_front();
}