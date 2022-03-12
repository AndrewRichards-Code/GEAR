#pragma once
#include "gear_core_common.h"
#include "Core/Timer.h"

namespace gear
{
namespace ui
{
namespace panels
{
	class GEAR_API Panel
	{
	public:
		enum class Type : uint32_t
		{
			UNKNOWN,
			CONTENT_BROWSER,
			CONTENT_EDITOR,
			MATERIAL,
			PROJECT,
			PROPERTIES,
			SCENE_HIERARCHY,
			VIEWPORT,
		};

	protected:
		Panel() = default;
		virtual ~Panel() = default;

	public:
		virtual void Update(gear::core::Timer timer) {};
		virtual void Draw() = 0;
		inline const bool& IsOpen() const { return m_Open; }
		inline const Type& GetPanelType() const { return m_Type; }

	protected:
		Type m_Type = Type::UNKNOWN;
		bool m_Open = true;
	};
}
}
}