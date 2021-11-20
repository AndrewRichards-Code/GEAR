#pragma once
#include "SceneHierarchyPanel.h"

namespace gearbox
{
namespace panels
{
	class PropertiesPanel final : public Panel
	{
		//enums/structs
	public:
		struct CreateInfo
		{
			void* null;
		};

		//Methods
	public:
		PropertiesPanel(CreateInfo* pCreateInfo);
		~PropertiesPanel();

		void Draw() override;

		inline CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	private:
		CreateInfo m_CI;
	};
}
}
