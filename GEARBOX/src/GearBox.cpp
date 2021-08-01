#include "GEARBOX.h"
#include "gear_core.h"
#include "UIContext.h"

using namespace gearbox::imgui;

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
	mainWindowCI.maximised = true;
	mainWindowCI.vSync = true;
	mainWindowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_4_BIT;
	mainWindowCI.graphicsDebugger = debug::GraphicsDebugger::DebuggerType::PIX;
	Ref<Window> mainWindow = CreateRef<Window>(&mainWindowCI);

	UIContext::CreateInfo uiContextCI;
	uiContextCI.window = mainWindow;
	Ref<UIContext>uiContext = CreateRef<UIContext>(&uiContextCI);

	CommandPool::CreateInfo cmdPoolCI;
	cmdPoolCI.debugName = "CommandPool";
	cmdPoolCI.pContext = mainWindow->GetContext();
	cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	cmdPoolCI.queueType = CommandPool::QueueType::GRAPHICS;
	Ref<CommandPool> cmdPool = CommandPool::Create(&cmdPoolCI);

	CommandBuffer::CreateInfo cmdBufferCI;
	cmdBufferCI.debugName = "CommandBuffers";
	cmdBufferCI.pCommandPool = cmdPool;
	cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdBufferCI.commandBufferCount = 3;
	Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

	Fence::CreateInfo fenceCI;
	fenceCI.debugName = "DrawFence";
	fenceCI.device = mainWindow->GetDevice();
	fenceCI.signaled = true;
	fenceCI.timeout = UINT64_MAX;
	std::vector<Ref<Fence>>draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
	Semaphore::CreateInfo acquireSemaphoreCI = { "AcquireSeamphore", mainWindow->GetDevice() };
	Semaphore::CreateInfo submitSemaphoreCI = { "SubmitSeamphore", mainWindow->GetDevice() };
	std::vector<Ref<Semaphore>>acquire = { Semaphore::Create(&acquireSemaphoreCI), Semaphore::Create(&acquireSemaphoreCI) };
	std::vector<Ref<Semaphore>>submit = { Semaphore::Create(&submitSemaphoreCI), Semaphore::Create(&submitSemaphoreCI) };
	uint32_t frameIndex = 0;
	uint32_t frameCount = 0;
	float r = 1.00f;
	float g = 0.00f;
	float b = 0.00f;
	bool windowResize = false;

	while (!mainWindow->Closed())
	{
		uiContext->NewFrame();

		ImGui::NewFrame();
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

			float colours[3] = { r,g,b };
			ImGui::ColorPicker3("Clear Colour", colours);
			r = colours[0];
			g = colours[1];
			b = colours[2];

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		// Rendering
		ImGui::Render();

		if (mainWindow->Resized())
		{
			frameIndex = 0;
			mainWindow->Resized() = false;

			cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

			draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
			acquire = { Semaphore::Create(&acquireSemaphoreCI), Semaphore::Create(&acquireSemaphoreCI) };
			submit = { Semaphore::Create(&submitSemaphoreCI), Semaphore::Create(&submitSemaphoreCI) };
		}

		draws[frameIndex]->Wait();

		cmdBuffer->Reset(frameIndex, false);
		cmdBuffer->Begin(frameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
		cmdBuffer->BeginRenderPass(frameIndex, mainWindow->GetRenderSurface()->GetHDRFramebuffers()[frameIndex], { {r, g, b, 1.0f}, {0.0f, 0}, {r, g, b, 1.0f} });

		uiContext->RenderDrawData(cmdBuffer, frameIndex, ImGui::GetDrawData());

		cmdBuffer->EndRenderPass(frameIndex);
		cmdBuffer->End(frameIndex);

		cmdBuffer->Present({ 0, 1 }, mainWindow->GetSwapchain(), draws, acquire, submit, windowResize);
		frameIndex = (frameIndex + 1) % 2;
		frameCount++;

		mainWindow->Update();
		mainWindow->CalculateFPS();
	}
	mainWindow->GetContext()->DeviceWaitIdle();
}