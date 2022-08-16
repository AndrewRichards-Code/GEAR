#include "gear_core_common.h"
#include "Core/ApplicationContext.h"

using namespace gear;
using namespace core;

using namespace miru;
using namespace base;

ApplicationContext::ApplicationContext(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	GraphicsAPI::SetAPI(m_CI.commandLineOptions.api, true);
	GraphicsAPI::AllowSetName();
	GraphicsAPI::LoadGraphicsDebugger(m_CI.commandLineOptions.graphicsDebugger);

#ifdef _DEBUG
	bool debugBuild = true;
#else
	bool debugBuild = false;
#endif
	m_CI.applicationName += ": x64";
	m_CI.applicationName += debugBuild ? "/Debug" : "";
	m_CI.applicationName += GraphicsAPI::IsD3D12() ? " - D3D12" : ": - Vulkan";

	Context::CreateInfo contextCI;
	contextCI.applicationName = m_CI.applicationName;
	contextCI.debugValidationLayers = m_CI.commandLineOptions.debugValidationLayers;
	contextCI.extensions = m_CI.extensions;
	contextCI.deviceDebugName = "GEAR_CORE_Context";
	m_Context = Context::Create(&contextCI);
}

ApplicationContext::~ApplicationContext()
{
	
}