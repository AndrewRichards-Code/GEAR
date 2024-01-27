#include "gear_core_common.h"
#include "Graphics/DebugRender.h"
#include "Graphics/AllocatorManager.h"
#include "Objects/Camera.h"

using namespace gear;
using namespace graphics;

void* DebugRender::s_Device = nullptr;
Ref<objects::Camera> DebugRender::s_Camera = nullptr;
Ref<Uniformbuffer<UniformBufferStructures::DebugProbeInfo>> DebugRender::s_DebugProbeInfo = nullptr;
Ref<Storagebuffer<UniformBufferStructures::Model>> DebugRender::s_DebugModelMatricesSB = nullptr;
std::vector<UniformBufferStructures::Model> DebugRender::s_DebugModelMatrices = {};

Ref<objects::Camera>& DebugRender::GetCamera()
{
	using namespace objects;
	if (!s_Camera && s_Device)
	{
		Camera::CreateInfo cameraCI;
		cameraCI.debugName = "GEAR_CORE_Camera: GEAR_CORE_DebugRender";
		cameraCI.device = s_Device;
		cameraCI.projectionType = Camera::ProjectionType::PERSPECTIVE;
		cameraCI.perspectiveParams = { mars::DegToRad(120.0), 1.0f, 0.001f, 1.0f };
		s_Camera = CreateRef<Camera>(&cameraCI);
	}

	return s_Camera;
}

Ref<Uniformbuffer<UniformBufferStructures::DebugProbeInfo>>& DebugRender::GetDebugProbeInfo()
{
	if (!s_DebugProbeInfo && s_Device)
	{
		UniformBufferStructures::DebugProbeInfo zero = { 0 };
		zero.maxDepth = 1.0f;
		Uniformbuffer<UniformBufferStructures::DebugProbeInfo>::CreateInfo ubCI;
		ubCI.debugName = "GEAR_CORE_DebugProbeInfo: GEAR_CORE_DebugRender";
		ubCI.device = s_Device;
		ubCI.data = &zero;
		s_DebugProbeInfo = CreateRef<Uniformbuffer<UniformBufferStructures::DebugProbeInfo>>(&ubCI);
	}

	return s_DebugProbeInfo;
}

Ref<Storagebuffer<UniformBufferStructures::Model>>& DebugRender::GetDebugModelMatricesStoragebuffer()
{
	bool create = !s_DebugModelMatricesSB && !s_DebugModelMatrices.empty() && s_Device;
	bool sameSize = false;

	if (s_DebugModelMatricesSB)
	{
		sameSize = s_DebugModelMatricesSB->GetCreateInfo().count == s_DebugModelMatrices.size();
		create |= !sameSize;
	}

	if (create)
	{
		Storagebuffer<UniformBufferStructures::Model>::CreateInfo sbCI;
		sbCI.debugName = "GEAR_CORE_DebugModelMatrices: GEAR_CORE_DebugRender";
		sbCI.device = s_Device;
		sbCI.data = s_DebugModelMatrices.data();
		sbCI.count = s_DebugModelMatrices.size();
		s_DebugModelMatricesSB = CreateRef<Storagebuffer<UniformBufferStructures::Model>>(&sbCI);
	}

	if (sameSize)
	{
		s_DebugModelMatricesSB->m_Data = s_DebugModelMatrices;
	}

	return s_DebugModelMatricesSB;
}
