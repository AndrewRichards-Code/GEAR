#pragma once
#include "SceneHierarchyPanel.h"
#include "Scene/Entity.h"

namespace gear
{
namespace ui
{
namespace panels
{
	class GEAR_API PropertiesPanel final : public Panel
	{
		//enums/structs
	public:

		//Methods
	public:
		PropertiesPanel();
		~PropertiesPanel();

		void Draw() override;

		//Members
	private:
		bool m_UseSelectedEntity = true;
		scene::Entity entity;
	};
}
}
}
