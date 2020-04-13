#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "gear_core_common.h"

namespace GEAR {
namespace GRAPHICS {

#define MAX_KEYS 1024 
#define MAX_BUTTONS 32 
#define MAX_AXES 6
#define MAX_JOY_BUTTONS 16

class Window
{
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
	std::string m_Title;
	int m_Width, m_Height;
	int m_InitWidth, m_InitHeight;
	GLFWwindow* m_Window;
	const GLFWvidmode* m_Mode;
	bool m_Fullscreen = false;
	bool m_VSync = true;
	int m_AntiAliasingValue = 0;

	double m_CurrentTime, m_PreviousTime = 0.0, m_DeltaTime, m_FPS;

	//Inputs
	bool m_Keys[MAX_KEYS];
	bool m_MouseButtons[MAX_BUTTONS];
	double mx, my;

	bool m_JoyButtons[MAX_JOY_BUTTONS];
	double m_xJoy1, m_yJoy1, m_xJoy2, m_yJoy2, m_xJoy3, m_yJoy3;

public:
	Window(std::string title, int width, int height, int antiAliasingValue, bool vsync, bool fullscreen);
	virtual ~Window();

	void Update();
	bool Closed() const;
	void Fullscreen();
	void UpdateVSync(bool vSync);
	void CalculateFPS();
	inline miru::Ref<miru::crossplatform::Context> GetContext() { return m_Context; };
	inline miru::Ref<miru::crossplatform::Swapchain> GetSwapchain() { return m_Swapchain; };
	inline void* GetDevice() { return m_Context->GetDevice(); }
	inline miru::Ref<miru::crossplatform::RenderPass> GetRenderPass() { return m_RenderPass; }
	inline miru::Ref<miru::crossplatform::ImageView> GetSwapchainImageView(size_t index) { return m_Swapchain->m_SwapchainImageViews[index]; }
	inline miru::Ref<miru::crossplatform::ImageView> GetSwapchainDepthImageView() { return m_DepthImageView; }
	inline miru::Ref<miru::crossplatform::Framebuffer>* GetFramebuffers() { return m_Framebuffers; }

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline float GetRatio() const { return ((float)m_Width / (float)m_Height); }
	inline std::string GetTitle() const { return m_Title; }
	std::string GetGraphicsAPIVersion() const;
	std::string GetDeviceName() const;
	template<typename T>
	inline std::string GetFPSString() const { return std::to_string(static_cast<T>(m_FPS)); }
	inline std::string GetAntiAliasingValue() const { return std::to_string(m_AntiAliasingValue); }

	bool IsKeyPressed(unsigned int keycode) const;
	bool IsMouseButtonPressed(unsigned int button) const;
	void GetMousePosition(double& x, double& y) const;
	bool IsJoyButtonPressed(unsigned int button) const;
	void GetJoyAxes(double& x1, double& y1, double& x2, double& y2, double& x3, double& y3) const;

private:
	bool Init();
	void CreateFramebuffer();
	static void window_resize(GLFWwindow* window, int width, int height);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void joystick_callback(int joy, int event);
};
}
}