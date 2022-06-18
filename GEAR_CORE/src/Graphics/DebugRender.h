#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace objects
	{
		class Camera;
	}
	namespace graphics
	{
		class GEAR_API DebugRender
		{
			DebugRender() = delete;
			~DebugRender() = delete;

		private:
			static void* s_Device;
			static Ref<objects::Camera> s_Camera;

		public:
			static void Initialise(void* device)
			{
				s_Device = device;
			}

			static void Uninitialise()
			{
				s_Camera = nullptr;
			}

			static Ref<objects::Camera>& GetCamera();

		};
	}
}