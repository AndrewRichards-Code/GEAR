#include "gearbox_common.h"
#include "GEARBOX.h"

#include "Panels/Panels.h"


using namespace gearbox;
using namespace panels;

using namespace gear;
using namespace animation;
using namespace audio;
using namespace core;
using namespace graphics;
using namespace objects;
using namespace scene;

using namespace miru;
using namespace miru::crossplatform;

using namespace mars;

Ref<Application> CreateApplication(int argc, char** argv)
{
	return CreateRef<GEARBOX>();
}

void GEARBOX::Run()
{
	GraphicsAPI::API api;
	debug::GraphicsDebugger::DebuggerType graphicsDebugger;

	std::string configFilepath = (std::filesystem::current_path() / std::filesystem::path("config.gbcf")).string();
	if (std::filesystem::exists(configFilepath))
	{
		nlohmann::json data;
		gear::core::LoadJsonFile(configFilepath, ".gbcf", "GEARBOX_CONFIG_FILE", data);

		api = (GraphicsAPI::API)data["options"]["api"];
		graphicsDebugger = (debug::GraphicsDebugger::DebuggerType)data["options"]["graphicsDebugger"];
		if (api == GraphicsAPI::API::UNKNOWN)
			api = GraphicsAPI::API::VULKAN;
	}

	Window::CreateInfo mainWindowCI;
	mainWindowCI.api = api;
	mainWindowCI.title = "GEARBOX";
	mainWindowCI.width = 1920;
	mainWindowCI.height = 1080;
	mainWindowCI.fullscreen = false;
	mainWindowCI.fullscreenMonitorIndex = 0;
	mainWindowCI.maximised = true;
	mainWindowCI.vSync = true;
	mainWindowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_4_BIT;
	mainWindowCI.graphicsDebugger = graphicsDebugger;
	Ref<Window> mainWindow = CreateRef<Window>(&mainWindowCI);

	AllocatorManager::CreateInfo mbmCI;
	mbmCI.pContext = mainWindow->GetContext();
	mbmCI.defaultBlockSize = Allocator::BlockSize::BLOCK_SIZE_128MB;
	AllocatorManager::Initialise(&mbmCI);

	Scene::CreateInfo sceneCI;
	sceneCI.debugName = "DefaultScene";
	sceneCI.nativeScriptDir = "res/scripts/";
	Ref<Scene> activeScene = CreateRef<Scene>(&sceneCI);
	
	Renderer::CreateInfo rendererCI;
	rendererCI.window = mainWindow;
	rendererCI.shouldCopyToSwapchian = false;
	rendererCI.shouldDrawExternalUI = true;
	rendererCI.shouldPresent = true;
	Ref<Renderer> renderer = CreateRef<Renderer>(&rendererCI);

	UIContext::CreateInfo uiContextCI;
	uiContextCI.window = mainWindow;
	Ref<UIContext>uiContext = CreateRef<UIContext>(&uiContextCI);

	std::vector<Ref<Panel>>& editorPanels = uiContext->GetEditorPanels();

	ViewportPanel::CreateInfo mainViewportCI = { renderer, uiContext };
	editorPanels.emplace_back(CreateRef<ViewportPanel>(&mainViewportCI));
	SceneHierarchyPanel::CreateInfo sceneHierarchyPanelCI = { activeScene, ref_cast<ViewportPanel>(editorPanels.back()) };
	editorPanels.emplace_back(CreateRef<SceneHierarchyPanel>(&sceneHierarchyPanelCI));
	PropertiesPanel::CreateInfo propertiesCI = { ref_cast<SceneHierarchyPanel>(editorPanels.back()) };
	editorPanels.emplace_back(CreateRef<PropertiesPanel>(&propertiesCI));
	ContentBrowserPanel::CreateInfo contentBrowserCI = { uiContext, std::filesystem::current_path() };
	editorPanels.emplace_back(CreateRef<ContentBrowserPanel>(&contentBrowserCI));

	core::Timer timer;
	while (!mainWindow->Closed())
	{
		uiContext->Draw();

		if (mainWindow->Resized())
		{
			renderer->ResizeRenderPipelineViewports(mainWindow->GetRenderSurface()->GetWidth(), mainWindow->GetRenderSurface()->GetHeight());
			mainWindow->Resized() = false;
		}

		Renderer::PFN_UIFunction UIDraw = [](const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, void* drawData, void* _this) -> void
		{
			UIContext::RenderDrawData(cmdBuffer, frameIndex, (ImDrawData*)drawData, (UIContext*)_this);
		};
		uiContext->GetPanel<SceneHierarchyPanel>()->GetScene()->OnUpdate(renderer, timer);
		renderer->SubmitRenderSurface(mainWindow->GetRenderSurface());
		renderer->SetUIFunction(UIDraw, ImGui::GetDrawData(), uiContext.get());
		renderer->Execute();

		mainWindow->Update();
		mainWindow->CalculateFPS();
	}
	mainWindow->GetContext()->DeviceWaitIdle();
}