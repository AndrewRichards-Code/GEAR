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
			PROPERTIES
		};

	protected:
		virtual ~Panel() = default;

	public:
		virtual void Draw() = 0;

	protected:
		Type m_Type = Type::UNKNOWN;
	};
}
}