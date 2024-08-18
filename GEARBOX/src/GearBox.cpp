#include "gearbox_common.h"
#include "gearbox.h"

//TODO: Find better place to put this!
extern "C"\
{\
__declspec(dllexport) extern const unsigned int D3D12SDKVersion = 613; \
__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; \
}

using namespace gear;
using namespace animation;
using namespace audio;
using namespace core;
using namespace graphics;
using namespace rendering;
using namespace objects;
using namespace project;
using namespace scene;
using namespace ui;
using namespace panels;

using namespace miru;
using namespace base;

using namespace mars;

Ref<Application> CreateApplication(int argc, char** argv)
{
	ApplicationContext::CreateInfo applicationCI;
	applicationCI.applicationName = "GEARBOX";
	applicationCI.extensions = ApplicationContext::DefaultExtensions();
	applicationCI.commandLineOptions = CommandLineOptions::GetCommandLineOptions(argc, argv).SetWorkingDirectory();

	std::string configFilepath = (std::filesystem::current_path() / std::filesystem::path("config.gbcf")).string();
	ConfigFile configFile;
	if (configFile.Load(configFilepath))
	{
		if (applicationCI.commandLineOptions.api == GraphicsAPI::API::UNKNOWN)
			applicationCI.commandLineOptions.api = configFile.GetOption<miru::base::GraphicsAPI::API>("api");
		if (applicationCI.commandLineOptions.debugValidationLayers == false)
			applicationCI.commandLineOptions.debugValidationLayers = configFile.GetOption<bool>("debugValidationLayers");
		if (applicationCI.commandLineOptions.graphicsDebugger == debug::GraphicsDebugger::DebuggerType::NONE)
			applicationCI.commandLineOptions.graphicsDebugger = configFile.GetOption<miru::debug::GraphicsDebugger::DebuggerType>("graphicsDebugger");
	}

	return CreateRef<GEARBOX>(ApplicationContext(&applicationCI));
}

GEARBOX::GEARBOX(const ApplicationContext& context)
	:Application(context) {}

void GEARBOX::Run()
{
	Window::CreateInfo mainWindowCI;
	mainWindowCI.applicationContext = m_ApplicationContext;
	mainWindowCI.width = 1920;
	mainWindowCI.height = 1080;
	mainWindowCI.fullscreen = false;
	mainWindowCI.fullscreenMonitorIndex = 0;
	mainWindowCI.maximised = true;
	mainWindowCI.vSync = true;
	mainWindowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_4_BIT;

	std::string configFilepath = (std::filesystem::current_path() / std::filesystem::path("config.gbcf")).string();
	ConfigFile configFile;
	if (configFile.Load(configFilepath))
		configFile.UpdateWindowCreateInfo(mainWindowCI);

	Ref<Window> mainWindow = CreateRef<Window>(&mainWindowCI);

	AllocatorManager::CreateInfo mbmCI;
	mbmCI.pContext = mainWindow->GetContext();
	mbmCI.defaultBlockSize = Allocator::BlockSize::BLOCK_SIZE_128MB;
	mbmCI.forceInitialisation = true;
	AllocatorManager::Initialise(&mbmCI);
	
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

	for (const Panel::Type& panelType : configFile.GetPanels())
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
		case Panel::Type::OUTPUT:
		{
			editorPanels.emplace_back(CreateRef<OutputPanel>());
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
		case Panel::Type::RENDERER_PROPERTIES:
		{
			editorPanels.emplace_back(CreateRef<RendererPropertiesPanel>());
			break;
		}
		case Panel::Type::SCENE_HIERARCHY:
		{
			Scene::CreateInfo sceneCI;
			sceneCI.debugName = "Default Scene";
			sceneCI.nativeScriptDir = std::filesystem::current_path() / "res/scripts/";
			SceneHierarchyPanel::CreateInfo sceneHierarchyPanelCI = { CreateRef<Scene>(&sceneCI) };
			editorPanels.emplace_back(CreateRef<SceneHierarchyPanel>(&sceneHierarchyPanelCI));
			ref_cast<SceneHierarchyPanel>(editorPanels.back())->UpdateWindowTitle();
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
			if (panel && panel->GetRenderer() != mainRenderer)
			{
				panel->GetRenderer()->Execute();
			}
		}

		mainRenderer->Execute();

		mainWindow->Update();
		mainWindow->CalculateFPS();
	}
	mainWindow->GetContext()->DeviceWaitIdle();
	AllocatorManager::Uninitialise();

	if (configFile.Load(configFilepath))
	{
		auto& configPanels = configFile.GetPanels();
		configPanels.clear();
		for (const auto& panel : uiContext->GetEditorPanels())
			configPanels.push_back(static_cast<uint32_t>(panel->GetPanelType()));
		configFile.Save();
	}
}