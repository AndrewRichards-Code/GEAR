#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace core
	{
		//Not valid for classes or structs.
		template<typename T>
		uint64_t GetHash(const T& object)
		{
			return static_cast<uint64_t>(std::hash<T>{}(object));
		}
	}
}