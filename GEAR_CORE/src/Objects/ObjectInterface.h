#pragma once
#include "gear_core_common.h"
#include "Transform.h"

namespace gear
{
namespace objects
{
	class GEAR_API ObjectInterface
	{
	public:
		struct CreateInfo
		{
			std::string	debugName;
			void* device;
		};

		virtual void Update(const Transform& transform) = 0;
	};
}
}
