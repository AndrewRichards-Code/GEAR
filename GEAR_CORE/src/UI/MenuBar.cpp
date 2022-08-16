#include "gear_core_common.h"
#include "UI/MenuBar.h"
#include "UI/UIContext.h"
#include "UI/Panels/Panels.h"
#include "UI/ComponentUI/ComponentUI.h"

#include "Core/Application.h"
#include "Core/ConfigFile.h"
#include "Core/FileDialog.h"
#include "Core/JsonFileHelper.h"

#include "Graphics/Rendering/Renderer.h"
#include "Graphics/Window.h"

#include "Build/Project.h"

using namespace gear;
using namespace build;
using namespace core;
using namespace graphics;
using namespace rendering;
using namespace scene;
using namespace ui;
using namespace panels;
using namespace componentui;

using namespace miru;
using namespace base;

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
	Ref<Window> window = UIContext::GetUIContext()->GetWindow();
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
	if (ShortcutPressed({ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_R }))
		DrawItemRecompileRenderPipelineShaders();
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

		ImGui::Separator();

		if (ImGui::MenuItem("Recompile RenderPipeline Shaders", "Ctrl+R"))
		{
			DrawItemRecompileRenderPipelineShaders();
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
			Scene::CreateInfo sceneCI;
			sceneCI.debugName = "DefaultScene";
			sceneCI.nativeScriptDir = "res/scripts/";
			Ref<scene::Scene> activeScene = CreateRef<scene::Scene>(&sceneCI);

			SceneHierarchyPanel::CreateInfo sceneHierarchyPanelCI = { activeScene };
			editorPanels.emplace_back(CreateRef<SceneHierarchyPanel>(&sceneHierarchyPanelCI));
		}
		if (ImGui::MenuItem("Material"))
		{
			editorPanels.emplace_back(CreateRef<MaterialPanel>());
		}
		if (ImGui::MenuItem("Project"))
		{
			editorPanels.emplace_back(CreateRef<ProjectPanel>());
		}
		if (ImGui::MenuItem("Properties"))
		{
			editorPanels.emplace_back(CreateRef<PropertiesPanel>());
		}
		if (ImGui::MenuItem("Renderer Properties"))
		{
			editorPanels.emplace_back(CreateRef<RendererPropertiesPanel>());
		}
		if (ImGui::MenuItem("Viewport"))
		{
			Renderer::CreateInfo rendererCI;
			rendererCI.window = UIContext::GetUIContext()->GetWindow();
			rendererCI.shouldCopyToSwapchian = false;
			rendererCI.shouldDrawExternalUI = false;
			rendererCI.shouldPresent = false;
			Ref<Renderer> renderer = CreateRef<Renderer>(&rendererCI);

			static uint32_t idx = 0;
			RenderSurface::CreateInfo renderSurfaceCI = rendererCI.window->GetRenderSurface()->GetCreateInfo();
			renderSurfaceCI.debugName = "Viewport " + std::to_string(idx++);
			renderer->SubmitRenderSurface(CreateRef<RenderSurface>(&renderSurfaceCI));

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
	std::string folderPath = FolderDialog_Browse();
	if (!folderPath.empty())
	{
		Project::CreateInfo projectCI = { UIContext::GetUIContext()->GetWindow(), std::filesystem::path(folderPath), true };
		Ref<Project> project = CreateRef<Project>(&projectCI);
		UIContext::GetUIContext()->SetProject(project);
	}
}

void MenuBar::DrawItemOpenProject()
{
	std::string folderPath = FolderDialog_Browse();
	if (!folderPath.empty())
	{
		Project::CreateInfo projectCI = { UIContext::GetUIContext()->GetWindow(), std::filesystem::path(folderPath), false };
		Ref<Project> project = CreateRef<Project>(&projectCI);
		UIContext::GetUIContext()->SetProject(project);
	}
}

void MenuBar::DrawItemNewScene()
{
	Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
	if (sceneHierarchyPanel)
	{
		Scene::CreateInfo sceneCI;
		sceneCI.debugName = "DefaultScene";
		sceneCI.nativeScriptDir = "res/scripts/";
		sceneHierarchyPanel->SetScene(CreateRef<scene::Scene>(&sceneCI));
		sceneHierarchyPanel->UpdateWindowTitle();
	}
}

void MenuBar::DrawItemOpenScene()
{
	Ref<SceneHierarchyPanel> sceneHierarchyPanel = UIContext::GetUIContext()->GetEditorPanelsByType<SceneHierarchyPanel>()[0];
	if (sceneHierarchyPanel)
	{
		std::string filepath = FileDialog_Open("GEAR Scene file", "*.gsf");

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
		std::string filepath = FileDialog_Save("GEAR Scene file", "*.gsf");

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
		Ref<Window> mainWindow = UIContext::GetUIContext()->GetWindow();
		const Window::CreateInfo& mainWindowCI = mainWindow->GetCreateInfo();

		uint32_t monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors((int*)&monitorCount);

		std::vector<std::string> monitorNames;
		std::vector<mars::int4> monitorRects;
		monitorNames.resize(monitorCount);
		monitorRects.resize(monitorCount);
		for (uint32_t i = 0; i < monitorCount; i++)
		{
			GLFWmonitor*& monitor = monitors[i];
			std::string& monitorName = monitorNames[i];
			mars::int4& monitorRect = monitorRects[i];

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

		std::string configFilepath = (std::filesystem::current_path() / std::filesystem::path("config.gbcf")).string();
		ConfigFile config;
		static bool loaded = false;
		if (config.Load(configFilepath) && !loaded)
		{
			api						= config.GetOption<GraphicsAPI::API>("api");
			graphicsDebugger		= config.GetOption<debug::GraphicsDebugger::DebuggerType>("graphicsDebugger");
			windowedWidth			= config.GetOption<uint32_t>("windowedWidth");
			windowedHeight			= config.GetOption<uint32_t>("windowedHeight");
			fullscreen				= config.GetOption<bool>("fullscreen");
			fullscreenMonitorIndex	= config.GetOption<uint32_t>("fullscreenMonitorIndex");
			maximised				= config.GetOption<bool>("maximised");
			loaded					= true;
		}

		DrawDropDownMenu("API", api, 130.0f);
		DrawDropDownMenu("Graphics Debugger", graphicsDebugger, 130.0f);
		DrawUint32("Windowed Width", windowedWidth, 0, 3840, false ,130.0f);
		DrawUint32("Windowed Height", windowedHeight, 0, 2160, false, 130.0f);
		DrawCheckbox("Fullscreen", fullscreen, 130.0f);
		DrawDropDownMenu("Fullscreen Monitor", monitorNames, fullscreenMonitorIndex, 130.0f);
		DrawCheckbox("Maximised", maximised, 130.0f);

		if (ImGui::Button("Save Options", ImVec2(90, 0)))
		{
			config.SetOption("api", api);
			config.SetOption("graphicsDebugger", graphicsDebugger);
			config.SetOption("windowedWidth", windowedWidth);
			config.SetOption("windowedHeight", windowedHeight);
			config.SetOption("fullscreen", fullscreen);
			config.SetOption("fullscreenMonitorIndex", fullscreenMonitorIndex);
			config.SetOption("maximised", maximised);
			config.Save();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close", ImVec2(90, 0)))
		{
			loaded = false;
			ImGui::CloseCurrentPopup();
			m_PopupWindowFunction = nullptr;
		}
		ImGui::SameLine();
		if (ImGui::Button("Restart Editor", ImVec2(90, 0)))
		{
			DrawItemExit();
			Application::IsActive() = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Exit Editor", ImVec2(90, 0)))
		{
			DrawItemExit();
		}
		ImGui::EndPopup();
	}
}

void MenuBar::DrawItemRecompileRenderPipelineShaders()
{
	Ref<ViewportPanel> viewportPanel = UIContext::GetUIContext()->GetEditorPanelsByType<ViewportPanel>()[0];
	if (viewportPanel)
	{
		viewportPanel->GetCreateInfo().renderer->RecompileRenderPipelineShaders();
	}
}
