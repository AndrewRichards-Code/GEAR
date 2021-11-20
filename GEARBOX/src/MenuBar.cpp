#include "gearbox_common.h"
#include "MenuBar.h"
#include "Panels/Panels.h"
#include "ComponentUI/ComponentUI.h"

#define GEAR_EXTERNAL_ENTRY_POINT
#include "GEARBOX.h"

using namespace gearbox;
using namespace panels;

using namespace miru;
using namespace crossplatform;

MenuBar::MenuBar()
{
}

MenuBar::~MenuBar()
{
}

void MenuBar::Draw()
{
	ProcessShortcuts();

	if (ImGui::BeginMenuBar())
	{
		DrawMenuFile();
		DrawMenuWindows();
		DrawMenuSettings();
		DrawMenuHelp();

		ImGui::EndMenuBar();
	}

	if (m_PopupWindowFunction)
		(this->*m_PopupWindowFunction)();
}

bool MenuBar::ShortcutPressed(const std::vector<uint32_t>& keycodes)
{
	Ref<gear::graphics::Window> window = UIContext::GetUIContext()->GetWindow();
	window->Update();

	std::vector<uint32_t> _keycodes = keycodes;
	std::vector<uint32_t> pressedKeycodes;
	for (uint32_t i = 0; i < MAX_KEYS; i++)
	{
		if (window->IsKeyPressed(static_cast<unsigned int>(i)))
			pressedKeycodes.push_back(i);
	}
	std::sort(pressedKeycodes.begin(), pressedKeycodes.end());
	std::sort(_keycodes.begin(), _keycodes.end());
	return pressedKeycodes == _keycodes;
}

void MenuBar::ProcessShortcuts()
{
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_N }))
		DrawItemNewScene();
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_O }))
		DrawItemOpenScene();
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_S }))
		DrawItemSaveScene();
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_S }) || m_SaveToSaveAs)
		DrawItemSaveSceneAs();
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_N }))
		DrawItemNewProject();
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_O }))
		DrawItemOpenProject();
}

void MenuBar::DrawMenuFile()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::BeginMenu("New"))
		{
			if (ImGui::MenuItem("Project...", "Ctrl+Shift+N"))
				DrawItemNewProject();
			if (ImGui::MenuItem("Scene...", "Ctrl+N"))
				DrawItemNewScene();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Open"))
		{
			if (ImGui::MenuItem("Project...", "Ctrl+Shift+O"))
				DrawItemOpenProject();
			if (ImGui::MenuItem("Scene...", "Ctrl+O"))
				DrawItemOpenScene();

			ImGui::EndMenu();
		}
		
		ImGui::Separator();
		
		if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
			DrawItemSaveScene();
		if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S") || m_SaveToSaveAs)
			DrawItemSaveScene();

		ImGui::Separator();

		if (ImGui::MenuItem("Exit", "Alt+F4"))
			DrawItemExit();

		ImGui::EndMenu();
	}
}

void MenuBar::DrawMenuSettings()
{
	if (ImGui::BeginMenu("Settings"))
	{
		if (ImGui::MenuItem("GEARBOX Options"))
		{
			m_PopupWindowFunction = &MenuBar::DrawItemGEARBOXOptions;
		}
		ImGui::EndMenu();
	}
}

void MenuBar::DrawMenuWindows()
{
	if (ImGui::BeginMenu("Windows"))
	{
		std::vector<Ref<Panel>>& editorPanels = UIContext::GetUIContext()->GetEditorPanels();
		if (ImGui::MenuItem("Content Browser"))
		{
			ContentBrowserPanel::CreateInfo contentBrowserCI = { std::filesystem::current_path() };
			editorPanels.emplace_back(CreateRef<ContentBrowserPanel>(&contentBrowserCI));
		}
		if (ImGui::MenuItem("Content Editor"))
		{
			ContentEditorPanel::CreateInfo contentEditorCI = { std::filesystem::current_path() };
			editorPanels.emplace_back(CreateRef<ContentEditorPanel>(&contentEditorCI));
		}
		if (ImGui::MenuItem("Scene Hierarchy"))
		{
			gear::scene::Scene::CreateInfo sceneCI;
			sceneCI.debugName = "DefaultScene";
			sceneCI.nativeScriptDir = "res/scripts/";
			Ref<gear::scene::Scene> activeScene = CreateRef<gear::scene::Scene>(&sceneCI);

			SceneHierarchyPanel::CreateInfo sceneHierarchyPanelCI = { activeScene };
			editorPanels.emplace_back(CreateRef<SceneHierarchyPanel>(&sceneHierarchyPanelCI));
		}
		if (ImGui::MenuItem("Project"))
		{
			editorPanels.emplace_back(CreateRef<ProjectPanel>());
		}
		if (ImGui::MenuItem("Properties"))
		{
			PropertiesPanel::CreateInfo propertiesCI = { 0 };
			editorPanels.emplace_back(CreateRef<PropertiesPanel>(&propertiesCI));
		}
		if (ImGui::MenuItem("Viewport"))
		{
			gear::graphics::Renderer::CreateInfo rendererCI;
			rendererCI.window = UIContext::GetUIContext()->GetWindow();
			rendererCI.shouldCopyToSwapchian = false;
			rendererCI.shouldDrawExternalUI = false;
			rendererCI.shouldPresent = false;
			Ref<gear::graphics::Renderer> renderer = CreateRef<gear::graphics::Renderer>(&rendererCI);

			static uint32_t idx = 0;
			gear::graphics::RenderSurface::CreateInfo renderSurfaceCI = rendererCI.window->GetRenderSurface()->GetCreateInfo();;
			renderSurfaceCI.debugName = "Viewport " + std::to_string(idx++);
			renderer->SubmitRenderSurface(CreateRef<gear::graphics::RenderSurface>(&renderSurfaceCI));

			ViewportPanel::CreateInfo viewportCI = { renderer };
			editorPanels.emplace_back(CreateRef<ViewportPanel>(&viewportCI));
		}
		ImGui::EndMenu();
	}
}

void MenuBar::DrawMenuHelp()
{
	if (ImGui::BeginMenu("Help"))
	{
		if (ImGui::MenuItem("About", nullptr, false))
		{
			m_PopupWindowFunction = &MenuBar::DrawItemAbout;
		}
		ImGui::EndMenu();
	}
}

void MenuBar::DrawItemNewProject()
{
	std::string folderPath = gear::core::FolderDialog_Browse();
	if (!folderPath.empty())
	{
		gear::build::Project::CreateInfo projectCI = { UIContext::GetUIContext()->GetWindow(), std::filesystem::path(folderPath), true };
		Ref<gear::build::Project> project = CreateRef<gear::build::Project>(&projectCI);
		UIContext::GetUIContext()->SetProject(project);
	}
}

void MenuBar::DrawItemOpenProject()
{
	std::string folderPath = gear::core::FolderDialog_Browse();
	if (!folderPath.empty())
	{
		gear::build::Project::CreateInfo projectCI = { UIContext::GetUIContext()->GetWindow(), std::filesystem::path(folderPath), false };
		Ref<gear::build::Project> project = CreateRef<gear::build::Project>(&projectCI);
		UIContext::GetUIContext()->SetProject(project);
	}
}

void MenuBar::DrawItemNewScene()
{
	Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
	if (sceneHierarchyPanel)
	{
		gear::scene::Scene::CreateInfo sceneCI;
		sceneCI.debugName = "DefaultScene";
		sceneCI.nativeScriptDir = "res/scripts/";
		sceneHierarchyPanel->SetScene(CreateRef<gear::scene::Scene>(&sceneCI));
		sceneHierarchyPanel->UpdateWindowTitle();
	}
}

void MenuBar::DrawItemOpenScene()
{
	Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
	if (sceneHierarchyPanel)
	{
		std::string filepath = gear::core::FileDialog_Open("GEAR Scene file", "*.gsf");

		if (!filepath.empty())
		{
			sceneHierarchyPanel->GetScene()->LoadFromFile(filepath, UIContext::GetUIContext()->GetWindow());
			sceneHierarchyPanel->UpdateWindowTitle();
		}
	}
}

void MenuBar::DrawItemSaveScene()
{
	Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
	if (sceneHierarchyPanel)
	{
		if (!sceneHierarchyPanel->GetScene()->GetFilepath().empty())
		{
			sceneHierarchyPanel->GetScene()->SaveToFile("");
			sceneHierarchyPanel->UpdateWindowTitle();
		}
		else
		{
			m_SaveToSaveAs = true;
		}
	}
}

void MenuBar::DrawItemSaveSceneAs()
{
	Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
	if (sceneHierarchyPanel)
	{
		std::string filepath = gear::core::FileDialog_Save("GEAR Scene file", "*.gsf");

		if (!filepath.empty())
		{
			sceneHierarchyPanel->GetScene()->SaveToFile(filepath);
			sceneHierarchyPanel->UpdateWindowTitle();
		}
	}
	m_SaveToSaveAs = false;
}

void MenuBar::DrawItemExit()
{
	UIContext::GetUIContext()->GetWindow()->Close();
};

void MenuBar::DrawItemAbout()
{
	if (!ImGui::IsPopupOpen("About"))
		ImGui::OpenPopup("About");

	if (ImGui::BeginPopupModal("About"))
	{
		ImGui::Text("GEARBOX - Version: %d.%d.%d %s", GEAR_VERSION_MAJOR, GEAR_VERSION_MINOR, GEAR_VERSION_PATCH, GEAR_VERSION_TYPE_STR);
		std::string gpu = "GPU: " + UIContext::GetUIContext()->GetWindow()->GetDeviceName();
		ImGui::Text(gpu.c_str());
		std::string api = "API: " + UIContext::GetUIContext()->GetWindow()->GetGraphicsAPIVersion();
		ImGui::Text(api.c_str());
		if (ImGui::Button("Close", ImVec2(80, 0)))
		{
			ImGui::CloseCurrentPopup();
			m_PopupWindowFunction = nullptr;
		}
		ImGui::EndPopup();
	}
}

void MenuBar::DrawItemGEARBOXOptions()
{
	if (!ImGui::IsPopupOpen("GEARBOX Options"))
		ImGui::OpenPopup("GEARBOX Options");

	if (ImGui::BeginPopupModal("GEARBOX Options"))
	{
		const gear::graphics::Window::CreateInfo& mainWindowCI = UIContext::GetUIContext()->GetWindow()->GetCreateInfo();

		uint32_t monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors((int*)&monitorCount);

		std::vector<std::string> monitorNames;
		std::vector<mars::Int4> monitorRects;
		monitorNames.resize(monitorCount);
		monitorRects.resize(monitorCount);
		for (uint32_t i = 0; i < monitorCount; i++)
		{
			GLFWmonitor*& monitor = monitors[i];
			std::string& monitorName = monitorNames[i];
			mars::Int4& monitorRect = monitorRects[i];

			monitorName = glfwGetWin32Adapter(monitor);
			monitorName = arc::ToLower(monitorName.substr(4));
			monitorName.replace(0, 1, arc::ToUpper(std::string(1, monitorName[0])));
			monitorName.insert(monitorName.size() - 1, 1, ' ');

			glfwGetMonitorPos(monitor, (int*)&monitorRect.x, (int*)&monitorRect.y);

			const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[i]);
			monitorRect.z = videoMode->width;
			monitorRect.w = videoMode->height;

			monitorName += " (" + std::to_string(monitorRect.z) + " x " + std::to_string(monitorRect.w) + ")";
			monitorName += " at (" + std::to_string(monitorRect.x) + ", " + std::to_string(monitorRect.y) + ")";
		}

		static GraphicsAPI::API api;
		static debug::GraphicsDebugger::DebuggerType graphicsDebugger;
		static uint32_t windowedWidth;
		static uint32_t windowedHeight;
		static bool fullscreen;
		static uint32_t fullscreenMonitorIndex;
		static bool maximised;

		nlohmann::json data;
		std::string configFilepath = (std::filesystem::current_path() / std::filesystem::path("config.gbcf")).string();
		static bool loaded = false;
		if (std::filesystem::exists(configFilepath) && !loaded)
		{
			gear::core::LoadJsonFile(configFilepath, ".gbcf", "GEARBOX_CONFIG_FILE", data);

			api = (GraphicsAPI::API)data["options"]["api"];
			graphicsDebugger = (debug::GraphicsDebugger::DebuggerType)data["options"]["graphicsDebugger"];
			windowedWidth = (uint32_t)data["options"]["windowedWidth"];
			windowedHeight = (uint32_t)data["options"]["windowedHeight"];
			fullscreen = (bool)data["options"]["fullscreen"];
			fullscreenMonitorIndex = (uint32_t)data["options"]["fullscreenMonitorIndex"];
			maximised = (bool)data["options"]["maximised"];
			loaded = true;
		}

		componentui::DrawDropDownMenu("API", { "Unknown", "D3D12", "Vulkan" }, api, 130.0f);
		componentui::DrawDropDownMenu("Graphics Debugger", { "None", "PIX", "RenderDoc" }, graphicsDebugger, 130.0f);
		componentui::DrawUint32("Windowed Width", windowedWidth, 0, 3840, false ,130.0f);
		componentui::DrawUint32("Windowed Height", windowedHeight, 0, 2160, false, 130.0f);
		componentui::DrawCheckbox("Fullscreen", fullscreen, 130.0f);
		componentui::DrawDropDownMenu("Fullscreen Monitor", monitorNames, fullscreenMonitorIndex, 130.0f);
		componentui::DrawCheckbox("Maximised", maximised, 130.0f);

		if (ImGui::Button("Save Options", ImVec2(90, 0)))
		{
			data["options"]["api"] = api;
			data["options"]["graphicsDebugger"] = graphicsDebugger;
			data["options"]["windowedWidth"] = windowedWidth;
			data["options"]["windowedHeight"] = windowedHeight;
			data["options"]["fullscreen"] = fullscreen;
			data["options"]["fullscreenMonitorIndex"] = fullscreenMonitorIndex;
			data["options"]["maximised"] = maximised;
			gear::core::SaveJsonFile(configFilepath, ".gbcf", "GEARBOX_CONFIG_FILE", data);
		}
		ImGui::SameLine();
		if (ImGui::Button("Restart", ImVec2(90, 0)))
		{
			GEARBOX::GetGEARBOX()->GetAllowReRun() = true;
			DrawItemExit();
		}
		ImGui::SameLine();
		if (ImGui::Button("Exit", ImVec2(90, 0)))
		{
			DrawItemExit();
		}

		if (ImGui::Button("Close", ImVec2(90, 0)))
		{
			loaded = false;
			ImGui::CloseCurrentPopup();
			m_PopupWindowFunction = nullptr;
		}
		ImGui::EndPopup();
	}
}
