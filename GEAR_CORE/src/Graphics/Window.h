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
	
		Ref<RenderSurface> m_RenderSurface;
		RenderSurface::CreateInfo m_RenderSurfaceCI;
	
		//Window
		CreateInfo m_CI;
		GLFWmonitor* m_Monitor;
		GLFWwindow* m_Window;
		const GLFWvidmode* m_Mode;
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
		bool Closed() const;
		void CalculateFPS();
	
		inline const Ref<miru::crossplatform::Context> GetContext() const { return m_RenderSurface->GetContext(); };
		inline const Ref<miru::crossplatform::Swapchain>& GetSwapchain() const { return m_RenderSurface->GetSwapchain(); };
		inline void* GetDevice() const { return m_RenderSurface->GetDevice(); }
		inline const Ref<RenderSurface> GetRenderSurface() const { return m_RenderSurface; }
	
		inline const miru::crossplatform::GraphicsAPI::API& GetGraphicsAPI() const { m_RenderSurface->GetGraphicsAPI(); }
		inline bool IsD3D12() const { return miru::crossplatform::GraphicsAPI::IsD3D12(); }
		inline bool IsVulkan() const { return miru::crossplatform::GraphicsAPI::IsVulkan(); }
		inline int GetWidth() const { return m_RenderSurface->GetWidth(); }
		inline int GetHeight() const { return m_RenderSurface->GetHeight(); }
		inline float GetRatio() const { return m_RenderSurface->GetRatio(); }
		inline bool& Resized() const { return m_RenderSurface->Resized(); }
		inline std::string GetTitle() const { return m_CI.title; }
		inline std::string GetGraphicsAPIVersion() const { return m_RenderSurface->GetGraphicsAPIVersion(); };
		inline std::string GetDeviceName() const { return m_RenderSurface->GetDeviceName(); };
		template<typename T>
		inline std::string GetFPSString() const { return std::to_string(static_cast<T>(m_FPS)); }
		inline std::string GetAntiAliasingValue() const { return m_RenderSurface->GetAntiAliasingValue(); }
	
		bool IsKeyPressed(unsigned int keycode) const;
		bool IsMouseButtonPressed(unsigned int button) const;
		void GetMousePosition(double& x, double& y) const;
		bool IsJoyButtonPressed(unsigned int button) const;
		void GetJoyAxes(double& x1, double& y1, double& x2, double& y2, double& x3, double& y3) const;
		void GetScrollPosition(double& position) const;

		GLFWwindow* GetGLFWwindow() { return m_Window; }
	
	private:
		bool Init();
		static void window_resize(GLFWwindow* window, int width, int height);
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
		static void joystick_callback(int joy, int event);
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	};
}
}