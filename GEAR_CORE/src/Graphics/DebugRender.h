#pragma once
#include "gear_core_common.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/Storagebuffer.h"

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

			static std::vector<UniformBufferStructures::Model> s_DebugModelMatrices;
			static Ref<Storagebuffer<UniformBufferStructures::Model>> s_DebugModelMatricesSB;

		public:
			static void Initialise(void* device)
			{
				s_Device = device;
			}

			static void Uninitialise()
			{
				s_Camera = nullptr;
				s_DebugProbeInfo = nullptr;
				s_DebugModelMatricesSB = nullptr;
			}

			static Ref<objects::Camera>& GetCamera();

			static Ref<Uniformbuffer<UniformBufferStructures::DebugProbeInfo>>& GetDebugProbeInfo();

			static std::vector<UniformBufferStructures::Model>& GetDebugModelMatrices() { return s_DebugModelMatrices; }
			static Ref<Storagebuffer<UniformBufferStructures::Model>>& GetDebugModelMatricesStoragebuffer();
		};
	}
}