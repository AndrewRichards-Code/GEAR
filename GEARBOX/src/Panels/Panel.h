#pragma once
#include "gearbox_common.h"

namespace gearbox
{
namespace panels
{
	class Panel
	{
	public:
		enum class Type : uint32_t
		{
			UNKNOWN,
			VIEWPORT,
			SCENE_HIERARCHY,
			PROPERTIES,
			CONTENT_BROWSER,
			CONTENT_EDITOR
		};

	protected:
		Panel() = default;
		virtual ~Panel() = default;

	public:
		virtual void Draw() = 0;
		inline const bool& IsOpen() const { return m_Open; }
		inline const Type& GetPanelType() const { return m_Type; }

	protected:
		Type m_Type = Type::UNKNOWN;
		bool m_Open = true;
	};
}
}