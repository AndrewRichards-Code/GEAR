#pragma once
#include "gear_core_common.h"
#include "Core/CommandLineOptions.h"

namespace gear
{
	namespace core
	{
		class GEAR_API ApplicationContext
		{
			//struct/enum
		public:
			struct CreateInfo
			{
				std::string							applicationName;
				miru::base::Context::ExtensionsBit	extensions;
				CommandLineOptions					commandLineOptions;
			};

			//Methods
		public:
			ApplicationContext() = default;
			ApplicationContext(CreateInfo* pCreateInfo);
			~ApplicationContext();

			inline miru::base::ContextRef GetContext() { return m_Context; }

			inline const CreateInfo& GetCreateInfo() const { return m_CI; }
			inline const CreateInfo& GetCreateInfo() { return m_CI; }

			inline const CommandLineOptions& GetCommandLineOptions() const { return m_CI.commandLineOptions; }
			inline const CommandLineOptions& GetCommandLineOptions() { return m_CI.commandLineOptions; }

			static inline miru::base::Context::ExtensionsBit DefaultExtensions()
			{
				return miru::base::Context::ExtensionsBit::DYNAMIC_RENDERING
					| miru::base::Context::ExtensionsBit::SYNCHRONISATION_2
					| miru::base::Context::ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER;
			}

			//Members
		private:
			CreateInfo m_CI;

			miru::base::ContextRef m_Context;
		};
	}
}