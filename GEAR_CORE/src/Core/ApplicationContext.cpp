#include "gear_core_common.h"
#include "Core/ApplicationContext.h"

using namespace gear;
using namespace core;

using namespace miru;
using namespace base;

ApplicationContext::ApplicationContext(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_VSDebugOutput = CreateRef<arc::VisualStudioDebugOutput>();
	
	std::filesystem::path logFilepath;
	if (!m_CI.commandLineOptions.logFilepath.empty())
	{
		if (m_CI.commandLineOptions.logFilepath.is_absolute())
			logFilepath = m_CI.commandLineOptions.logFilepath;
		else
			logFilepath = m_CI.commandLineOptions.workingDirectory / m_CI.commandLineOptions.logFilepath;
	}
	else 
	{
		std::string logFilename = "GEAR_CORE_LOG_";
		logFilename += arc::GetDateAndTime_Filename();
		logFilename += ".txt";

		logFilepath = m_CI.commandLineOptions.workingDirectory / "Logs" / logFilename;
		std::filesystem::create_directory(logFilepath.parent_path());
	}
	m_VSDebugOutput->SetLogFile(logFilepath.string());

	//Default API
	if (m_CI.commandLineOptions.api == GraphicsAPI::API::UNKNOWN)
		m_CI.commandLineOptions.api = GraphicsAPI::API::VULKAN;

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
	m_CI.applicationName += GraphicsAPI::IsD3D12() ? " - D3D12" : " - Vulkan";
	m_CI.applicationName += m_CI.commandLineOptions.debugValidationLayers ? " - GPU Validation" : "";

	Context::CreateInfo contextCI;
	contextCI.applicationName = m_CI.applicationName;
	contextCI.debugValidationLayers = m_CI.commandLineOptions.debugValidationLayers;
	contextCI.extensions = m_CI.extensions;
	contextCI.deviceDebugName = "GEAR_CORE_Context";
	contextCI.pNext = nullptr;
	m_Context = Context::Create(&contextCI);
}

ApplicationContext::~ApplicationContext()
{
	
}