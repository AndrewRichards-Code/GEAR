#pragma once

#include "gear_core_common.h"
#include "Graphics/RenderSurface.h"

#define MAX_KEYS 1024 
#define MAX_BUTTONS 32 
#define MAX_AXES 6
#define MAX_JOY_BUTTONS 16

namespace gear 
{
namespace graphics 
{
	class Window
	{
	public:
		struct CreateInfo
		{
			miru::crossplatform::GraphicsAPI::API		api;
			std::string									title;
			uint32_t									width;
			uint32_t									height;
			bool										fullscreen;
			uint32_t									fullscreenMonitorIndex;
			bool										maximised;
			bool										vSync;
			miru::crossplatform::Image::SampleCountBit	samples;
			std::string									iconFilepath;
			miru::debug::GraphicsDebugger::DebuggerType graphicsDebugger;
		};
	
	private:
		friend struct GLFWwindow;

		//Context and Swapchain
		Ref<miru::crossplatform::Context> m_Context;
		miru::crossplatform::Context::CreateInfo m_ContextCI;
		Ref<miru::crossplatform::Swapchain> m_Swapchain;
		miru::crossplatform::Swapchain::CreateInfo m_SwapchainCI;
	
		//RenderSurface
		Ref<RenderSurface> m_RenderSurface;
		RenderSurface::CreateInfo m_RenderSurfaceCI;

		//SwapchainRenderPass and SwapchainFramebuffer
		Ref<miru::crossplatform::RenderPass> m_SwapchainRenderPass;
		miru::crossplatform::RenderPass::CreateInfo m_SwapchainRenderPassCI;
		Ref<miru::crossplatform::Framebuffer> m_SwapchainFramebuffers[2];
		miru::crossplatform::Framebuffer::CreateInfo m_SwapchainFramebufferCI;

		//Window
		CreateInfo m_CI;
		GLFWmonitor* m_Monitor;
		GLFWwindow* m_Window;
		const GLFWvidmode* m_Mode;
		uint32_t m_CurrentWidth, m_CurrentHeight;
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

		const CreateInfo& GetCreateInfo() { return m_CI; }
			
		void Update();
		void Close();
		bool Closed() const;
		void CalculateFPS();
		std::string GetGraphicsAPIVersion() const;
		std::string GetDeviceName() const ;
	
		bool IsKeyPressed(unsigned int keycode) const;
		bool IsMouseButtonPressed(unsigned int button) const;
		void GetMousePosition(double& x, double& y) const;
		bool IsJoyButtonPressed(unsigned int button) const;
		void GetJoyAxes(double& x1, double& y1, double& x2, double& y2, double& x3, double& y3) const;
		void GetScrollPosition(double& position) const;
	
	private:
		bool Init();
		void CreateSwapchainFramebuffer();

		static void window_resize(GLFWwindow* window, int width, int height);
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
		static void joystick_callback(int joy, int event);
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	public:
		inline GLFWwindow* GetGLFWwindow() { return m_Window; }
		inline const Ref<miru::crossplatform::Context> GetContext() const { return m_Context; };
		inline const Ref<miru::crossplatform::Swapchain>& GetSwapchain() const { return m_Swapchain; };
		inline void* GetDevice() const { return m_Context->GetDevice(); }
		inline const Ref<RenderSurface> GetRenderSurface() const { return m_RenderSurface; }
		inline const Ref<miru::crossplatform::RenderPass>& GetSwapchainRenderPass() const { return m_SwapchainRenderPass; }
		inline const Ref<miru::crossplatform::Framebuffer>* GetSwapchainFramebuffers() { return m_SwapchainFramebuffers; }

		inline const miru::crossplatform::GraphicsAPI::API& GetGraphicsAPI() const { return m_CI.api; }
		inline bool IsD3D12() const { return miru::crossplatform::GraphicsAPI::IsD3D12(); }
		inline bool IsVulkan() const { return miru::crossplatform::GraphicsAPI::IsVulkan(); }
		inline uint32_t GetWidth() const { return m_CurrentWidth; }
		inline uint32_t GetHeight() const { return m_CurrentHeight; }
		inline float GetRatio() const { return ((float)m_CurrentWidth / (float)m_CurrentHeight); }
		inline bool& Resized() const { return m_Swapchain->m_Resized; }
		inline std::string GetTitle() const { return m_CI.title; }
		template<typename T>
		inline std::string GetFPSString() const { return std::to_string(static_cast<T>(m_FPS)); }
		inline std::string GetAntiAliasingValue() const { return m_RenderSurface->GetAntiAliasingValue(); }
	};
}
}