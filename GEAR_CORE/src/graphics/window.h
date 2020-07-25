#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {

#define MAX_KEYS 1024 
#define MAX_BUTTONS 32 
#define MAX_AXES 6
#define MAX_JOY_BUTTONS 16

class Window
{
public:
	enum class MSAALevel : uint32_t
	{
		MSAA_1X = 0x00000001,
		MSAA_2X = 0x00000002,
		MSAA_4X = 0x00000004,
		MSAA_8X = 0x00000008
	};

	struct CreateInfo
	{
		miru::crossplatform::GraphicsAPI::API		api;
		std::string									title;
		uint32_t									width;
		uint32_t									height;
		bool										fullscreen;
		bool										vSync;
		miru::crossplatform::Image::SampleCountBit	samples;
		std::string									iconFilepath;
	};

private:
	friend struct GLFWwindow;

	//Context and Swapchain
	miru::Ref<miru::crossplatform::Context> m_Context;
	miru::crossplatform::Context::CreateInfo m_ContextCI;
	miru::Ref<miru::crossplatform::Swapchain> m_Swapchain;
	miru::crossplatform::Swapchain::CreateInfo m_SwapchainCI;

	//DepthImage
	miru::Ref<miru::crossplatform::MemoryBlock> m_DepthMB;
	miru::Ref<miru::crossplatform::Image> m_DepthImage;
	miru::crossplatform::Image::CreateInfo m_DepthImageCI;
	miru::Ref<miru::crossplatform::ImageView> m_DepthImageView;
	miru::crossplatform::ImageView::CreateInfo m_DepthImageViewCI;

	//RenderPass and Framebuffer
	miru::Ref<miru::crossplatform::RenderPass> m_RenderPass;
	miru::crossplatform::RenderPass::CreateInfo m_RenderPassCI;
	miru::Ref<miru::crossplatform::Framebuffer> m_Framebuffers[2];
	miru::crossplatform::Framebuffer::CreateInfo m_FramebufferCI;

	//Window
	CreateInfo m_CI;
	GLFWwindow* m_Window;
	const GLFWvidmode* m_Mode;
	int m_CurrentWidth, m_CurrentHeight;
	bool m_Fullscreen;
	double m_CurrentTime, m_PreviousTime = 0.0, m_DeltaTime, m_FPS;

	//Inputs
	bool m_Keys[MAX_KEYS];
	bool m_MouseButtons[MAX_BUTTONS];
	double mx, my;
	double m_ScrollPosition = 0.0;

	bool m_JoyButtons[MAX_JOY_BUTTONS];
	double m_xJoy1, m_yJoy1, m_xJoy2, m_yJoy2, m_xJoy3, m_yJoy3;

public:
	Window(CreateInfo* pCreateInfo);
	virtual ~Window();

	void Update();
	bool Closed() const;
	void Fullscreen();
	void CalculateFPS();

	inline const CreateInfo& GetCreateInfo() const { return m_CI; }
	inline const miru::Ref<miru::crossplatform::Context> GetContext() const { return m_Context; };
	inline const miru::Ref<miru::crossplatform::Swapchain>& GetSwapchain() const { return m_Swapchain; };
	inline void* GetDevice() const { return m_Context->GetDevice(); }
	inline const miru::Ref<miru::crossplatform::RenderPass>& GetRenderPass() const { return m_RenderPass; }
	inline const miru::Ref<miru::crossplatform::ImageView>& GetSwapchainImageView(size_t index) const { return m_Swapchain->m_SwapchainImageViews[index]; }
	inline const miru::Ref<miru::crossplatform::ImageView>& GetSwapchainDepthImageView() const { return m_DepthImageView; }
	inline const miru::Ref<miru::crossplatform::Framebuffer>* GetFramebuffers() { return m_Framebuffers; }

	inline const miru::crossplatform::GraphicsAPI::API& GetGraphicsAPI() const { return m_CI.api; }
	inline bool IsD3D12() const { return miru::crossplatform::GraphicsAPI::IsD3D12(); }
	inline bool IsVulkan() const { return miru::crossplatform::GraphicsAPI::IsVulkan(); }
	inline int GetWidth() const { return m_CurrentWidth; }
	inline int GetHeight() const { return m_CurrentHeight; }
	inline float GetRatio() const { return ((float)m_CurrentWidth / (float)m_CurrentHeight); }
	inline std::string GetTitle() const { return m_CI.title; }
	std::string GetGraphicsAPIVersion() const;
	std::string GetDeviceName() const;
	template<typename T>
	inline std::string GetFPSString() const { return std::to_string(static_cast<T>(m_FPS)); }
	inline std::string GetAntiAliasingValue() const { return std::to_string(static_cast<uint32_t>(m_CI.samples)); }

	bool IsKeyPressed(unsigned int keycode) const;
	bool IsMouseButtonPressed(unsigned int button) const;
	void GetMousePosition(double& x, double& y) const;
	bool IsJoyButtonPressed(unsigned int button) const;
	void GetJoyAxes(double& x1, double& y1, double& x2, double& y2, double& x3, double& y3) const;
	void GetScrollPosition(double& position) const;

private:
	bool Init();
	void CreateFramebuffer();
	static void window_resize(GLFWwindow* window, int width, int height);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void joystick_callback(int joy, int event);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};
}
}