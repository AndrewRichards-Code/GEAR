#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace core
	{
		struct CommandLineOptions
		{
			miru::base::GraphicsAPI::API				api;
			miru::debug::GraphicsDebugger::DebuggerType	graphicsDebugger;
			bool										debugValidationLayers;
			std::filesystem::path						workingDirectory;
			std::filesystem::path						projectDirectory;
			std::filesystem::path						sceneFilepath;

			CommandLineOptions()
			{
				api = miru::base::GraphicsAPI::API::UNKNOWN;
				graphicsDebugger = miru::debug::GraphicsDebugger::DebuggerType::NONE;
				debugValidationLayers = false;
				workingDirectory = std::filesystem::current_path();
				projectDirectory = std::filesystem::path();
				sceneFilepath = std::filesystem::path();
			}
			~CommandLineOptions() = default;

			static CommandLineOptions GetCommandLineOptions(int argc, char** argv)
			{
				CommandLineOptions commandLineOptions;
				for (int i = 0; i < argc; i++)
				{
					std::string arg = argv[i];
					auto FoundArg = [&](const std::string& value)->bool { return (arg.find(value) != std::string::npos); };
					auto RemoveArgLead = [&](const std::string& value)->std::string { return arg.substr(std::string(value).size()); };

					if (FoundArg("-d3d12") || FoundArg("-D3D12") || FoundArg("-dx12") || FoundArg("-DX12"))
						commandLineOptions.api = miru::base::GraphicsAPI::API::D3D12;
					else if (FoundArg("-vulkan") || FoundArg("-VULKAN") || FoundArg("-vk") || FoundArg("-VK"))
						commandLineOptions.api = miru::base::GraphicsAPI::API::VULKAN;
					else if (FoundArg("-pix") || FoundArg("-PIX"))
						commandLineOptions.graphicsDebugger = miru::debug::GraphicsDebugger::DebuggerType::PIX;
					else if (FoundArg("-rdc") || FoundArg("-RDC"))
						commandLineOptions.graphicsDebugger = miru::debug::GraphicsDebugger::DebuggerType::RENDER_DOC;
					else if (FoundArg("-debug"))
						commandLineOptions.debugValidationLayers = true;
					else if (FoundArg("-cwd:"))
						commandLineOptions.workingDirectory = RemoveArgLead("-cwd:");
					else if (FoundArg("-project:"))
						commandLineOptions.projectDirectory = RemoveArgLead("-project:");
					else if (FoundArg("-scene:") && FoundArg(".gsf"))
						commandLineOptions.projectDirectory = RemoveArgLead("-scene:");

				}
				return commandLineOptions;
			}

			CommandLineOptions& SetWorkingDirectory()
			{
				std::filesystem::current_path() = workingDirectory;
				return *this;
			}

		};
	}
}
