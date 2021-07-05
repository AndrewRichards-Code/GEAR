#pragma once
#include "gear_core_common.h"

namespace gear
{
namespace core
{
	class Application
	{
	public:
		Application() = default;
		virtual ~Application() = default;

		virtual void Run() = 0;
	};
}
}