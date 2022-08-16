#pragma once
#include "gear_core_common.h"
#include "Graphics/Window.h"

namespace gear
{
	namespace core
	{
		class GEAR_API ConfigFile
		{
		public:
			ConfigFile() = default;
			~ConfigFile() = default;

			bool Load(std::string& filepath);
			void Save();
			void UpdateWindowCreateInfo(graphics::Window::CreateInfo& windowCI);

			template<typename T>
			T GetOption(const std::string& member)
			{
				if (m_Data["options"].find(member) != m_Data["options"].end())
					return (T)m_Data["options"][member];
				else
					return T();
			}
			template<typename T>
			void SetOption(const std::string& member, const T& value)
			{
				if (m_Data["options"].find(member) != m_Data["options"].end())
					m_Data["options"][member] = value;
			}

			inline nlohmann::json& GetPanels() { return m_Data["panels"]; }
			inline nlohmann::json& GetData() { return m_Data; }

			
		private:
			std::string m_Filepath;
			nlohmann::json m_Data;
		};
	}
}
