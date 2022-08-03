#include "gear_core_common.h"
#include "UI/ConfigFile.h"
#include "Core/JsonFileHelper.h"

using namespace gear;
using namespace core;
using namespace ui;

bool ConfigFile::Load(std::string& filepath)
{
	if (std::filesystem::exists(filepath))
	{
		LoadJsonFile(filepath, ".gbcf", "GEARBOX_CONFIG_FILE", m_Data);
		m_Filepath = filepath;
		return true;
	}
	return false;
}

void ConfigFile::Save()
{
	SaveJsonFile(m_Filepath, ".gbcf", "GEARBOX_CONFIG_FILE", m_Data);
}

void ConfigFile::UpdateWindowCreateInfo(graphics::Window::CreateInfo& windowCI)
{
	windowCI.api					= GetOption<miru::base::GraphicsAPI::API>("api");
	windowCI.graphicsDebugger		= GetOption<miru::debug::GraphicsDebugger::DebuggerType>("graphicsDebugger");
	windowCI.width					= GetOption<uint32_t>("windowedWidth");
	windowCI.height					= GetOption<uint32_t>("windowedHeight");
	windowCI.fullscreen				= GetOption<bool>("fullscreen");
	windowCI.fullscreenMonitorIndex = GetOption<uint32_t>("fullscreenMonitorIndex");
	windowCI.maximised				= GetOption<bool>("maximised");
}
