#pragma once
#include "gearbox_common.h"

namespace gearbox
{
namespace panels
{
	class Panel
	{
	protected:
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

	protected:
		Type m_Type = Type::UNKNOWN;
		bool m_Open = true;
	};
}
}