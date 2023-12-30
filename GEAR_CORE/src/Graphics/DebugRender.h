#pragma once
#include "gear_core_common.h"
#include "Graphics/UniformBuffer.h"

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
			static Ref<Uniformbuffer<UniformBufferStructures::DebugProbeInfo>> s_DebugProbeInfo;

		public:
			static void Initialise(void* device)
			{
				s_Device = device;
			}

			static void Uninitialise()
			{
				s_Camera = nullptr;
				s_DebugProbeInfo = nullptr;
			}

			static Ref<objects::Camera>& GetCamera();
			static Ref<Uniformbuffer<UniformBufferStructures::DebugProbeInfo>>& GetDebugProbeInfo();

		};
	}
}