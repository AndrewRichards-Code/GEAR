#include "GEARBOX.h"
#include "gear_core.h"
#include "UIContext.h"
#include "Panels/ViewportPanel.h"

using namespace gearbox;
using namespace imgui;

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
	Window::CreateInfo mainWindowCI;
	mainWindowCI.api = GraphicsAPI::API::D3D12;
	mainWindowCI.title = "GEARBOX";
	mainWindowCI.width = 1920;
	mainWindowCI.height = 1080;
	mainWindowCI.fullscreen = false;
	mainWindowCI.fullscreenMonitorIndex = 0;
	mainWindowCI.maximised = false;
	mainWindowCI.vSync = true;
	mainWindowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_4_BIT;
	mainWindowCI.graphicsDebugger = debug::GraphicsDebugger::DebuggerType::PIX;
	Ref<Window> mainWindow = CreateRef<Window>(&mainWindowCI);

	AllocatorManager::CreateInfo mbmCI;
	mbmCI.pContext = mainWindow->GetContext();
	mbmCI.defaultBlockSize = Allocator::BlockSize::BLOCK_SIZE_128MB;
	AllocatorManager::Initialise(&mbmCI);

	UIContext::CreateInfo uiContextCI;
	uiContextCI.window = mainWindow;
	Ref<UIContext>uiContext = CreateRef<UIContext>(&uiContextCI);

	
	bool windowResize = false;

	Ref<Renderer> renderer = CreateRef<Renderer>(mainWindow);
	renderer->InitialiseRenderPipelines(mainWindow->GetRenderSurface());

	panels::ViewportPanel mainViewport;
	while (!mainWindow->Closed())
	{
		if (mainWindow->Resized())
		{
			renderer->ResizeRenderPipelineViewports(mainWindow->GetWidth(), mainWindow->GetHeight());
			mainWindow->Resized() = false;
		}

		uiContext->BeginFrame();
		uiContext->BeginDockspace();
		{
			mainViewport.Draw(mainWindow->GetRenderSurface(), uiContext);
		}
		uiContext->EndDockspace();
		uiContext->EndFrame();
		
		Renderer::PFN_UIFunction UIDraw = [](const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, void* drawData, void* _this) -> void
		{
			UIContext::RenderDrawData(cmdBuffer, frameIndex, (ImDrawData*)drawData, (UIContext*)_this);
		};

		renderer->SubmitRenderSurface(mainWindow->GetRenderSurface());
		renderer->SetUIFunction(UIDraw, ImGui::GetDrawData(), uiContext.get());
		renderer->Upload(true, false, false, false);
		renderer->Flush();
		renderer->Present(mainWindow->GetSwapchain(), windowResize);

		mainWindow->Update();
		mainWindow->CalculateFPS();
	}
	mainWindow->GetContext()->DeviceWaitIdle();
}