#include "gearbox_common.h"
#include "gearbox.h"

using namespace gear;
using namespace animation;
using namespace audio;
using namespace core;
using namespace graphics;
using namespace objects;
using namespace scene;
using namespace ui;
using namespace panels;

using namespace miru;
using namespace base;

using namespace mars;

GEARBOX* GEARBOX::s_App = nullptr;

Ref<Application> CreateApplication(int argc, char** argv)
{
	return CreateRef<GEARBOX>();
}

void GEARBOX::Run()
{
	s_App = this;

	while (m_AllowReRun)
		InternalRun();
	
	s_App = nullptr;
}

void GEARBOX::InternalRun()
{
	m_AllowReRun = false;

	Window::CreateInfo mainWindowCI;
	mainWindowCI.api = GraphicsAPI::API::VULKAN;;
	mainWindowCI.title = "GEARBOX";
	mainWindowCI.width = 1920;
	mainWindowCI.height = 1080;
	mainWindowCI.fullscreen = false;
	mainWindowCI.fullscreenMonitorIndex = 0;
	mainWindowCI.maximised = true;
	mainWindowCI.vSync = true;
	mainWindowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_4_BIT;
	mainWindowCI.graphicsDebugger = debug::GraphicsDebugger::DebuggerType::NONE;

	nlohmann::json data;
	std::string configFilepath = (std::filesystem::current_path() / std::filesystem::path("config.gbcf")).string();
	if (std::filesystem::exists(configFilepath))
	{
		gear::core::LoadJsonFile(configFilepath, ".gbcf", "GEARBOX_CONFIG_FILE", data);

		mainWindowCI.api = (GraphicsAPI::API)data["options"]["api"];
		mainWindowCI.graphicsDebugger = (debug::GraphicsDebugger::DebuggerType)data["options"]["graphicsDebugger"];
		mainWindowCI.width = (uint32_t)data["options"]["windowedWidth"];
		mainWindowCI.height = (uint32_t)data["options"]["windowedHeight"];
		mainWindowCI.fullscreen = (bool)data["options"]["fullscreen"];
		mainWindowCI.fullscreenMonitorIndex = (uint32_t)data["options"]["fullscreenMonitorIndex"];
		mainWindowCI.maximised = (bool)data["options"]["maximised"];
	}

	Ref<Window> mainWindow = CreateRef<Window>(&mainWindowCI);

	AllocatorManager::CreateInfo mbmCI;
	mbmCI.pContext = mainWindow->GetContext();
	mbmCI.defaultBlockSize = Allocator::BlockSize::BLOCK_SIZE_128MB;
	mbmCI.forceInitialisation = true;
	AllocatorManager::Initialise(&mbmCI);

	Scene::CreateInfo sceneCI;
	sceneCI.debugName = "DefaultScene";
	sceneCI.nativeScriptDir = "res/scripts/";
	Ref<Scene> activeScene = CreateRef<Scene>(&sceneCI);
	
	Renderer::CreateInfo mainRendererCI;
	mainRendererCI.window = mainWindow;
	mainRendererCI.shouldCopyToSwapchian = false;
	mainRendererCI.shouldDrawExternalUI = true;
	mainRendererCI.shouldPresent = true;
	Ref<Renderer> mainRenderer = CreateRef<Renderer>(&mainRendererCI);

	UIContext::CreateInfo uiContextCI;
	uiContextCI.window = mainWindow;
	Scope<UIContext> uiContext = CreateScope<UIContext>(&uiContextCI);
	mainRenderer->SubmitUIContext(uiContext.get());

	for (const Panel::Type& panelType : data["panels"])
	{
		std::vector<Ref<Panel>>& editorPanels = uiContext->GetEditorPanels();

		switch (panelType)
		{
		default:
		case Panel::Type::UNKNOWN:
			break;
		case Panel::Type::CONTENT_BROWSER:
		{
			ContentBrowserPanel::CreateInfo contentBrowserCI = { std::filesystem::current_path() };
			editorPanels.emplace_back(CreateRef<ContentBrowserPanel>(&contentBrowserCI));
			break;
		}
		case Panel::Type::CONTENT_EDITOR:
		{
			ContentEditorPanel::CreateInfo contentEditorCI = { std::filesystem::current_path() };
			editorPanels.emplace_back(CreateRef<ContentEditorPanel>(&contentEditorCI));
			break;
		}
		case Panel::Type::MATERIAL:
		{
			editorPanels.emplace_back(CreateRef<MaterialPanel>());
			break;
		}
		case Panel::Type::PROJECT:
		{
			editorPanels.emplace_back(CreateRef<ProjectPanel>());
			break;
		}
		case Panel::Type::PROPERTIES:
		{
			editorPanels.emplace_back(CreateRef<PropertiesPanel>());
			break;
		}
		case Panel::Type::SCENE_HIERARCHY:
		{
			SceneHierarchyPanel::CreateInfo sceneHierarchyPanelCI = { activeScene };
			editorPanels.emplace_back(CreateRef<SceneHierarchyPanel>(&sceneHierarchyPanelCI));
			break;
		}
		case Panel::Type::VIEWPORT:
		{
			ViewportPanel::CreateInfo mainViewportCI = { mainRenderer };
			editorPanels.emplace_back(CreateRef<ViewportPanel>(&mainViewportCI));
			break;
		}
		}
	}

	core::Timer timer;
	while (!mainWindow->Closed())
	{
		uiContext->Update(timer);
		uiContext->Draw();

		if (mainWindow->Resized())
		{
			mainWindow->Resized() = false;
		}

		for (auto& panel : uiContext->GetEditorPanelsByType<ViewportPanel>())
		{
			if (panel && panel->GetCreateInfo().renderer != mainRenderer)
			{
				panel->GetCreateInfo().renderer->Execute();
			}
		}

		mainRenderer->Execute();

		mainWindow->Update();
		mainWindow->CalculateFPS();
	}
	mainWindow->GetContext()->DeviceWaitIdle();
	AllocatorManager::Uninitialise();

	if (std::filesystem::exists(configFilepath))
	{
		gear::core::LoadJsonFile(configFilepath, ".gbcf", "GEARBOX_CONFIG_FILE", data);
		data["panels"].clear();
		for (const auto& panel : uiContext->GetEditorPanels())
		{
			data["panels"].push_back(static_cast<uint32_t>(panel->GetPanelType()));
		}
		gear::core::SaveJsonFile(configFilepath, ".gbcf", "GEARBOX_CONFIG_FILE", data);
	}
}