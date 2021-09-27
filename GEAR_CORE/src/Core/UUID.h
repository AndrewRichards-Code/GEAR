#pragma once
#include "gear_core_common.h"

namespace gear
{
namespace core
{
	class UUID
	{
	private:
		uint64_t m_UUID;

	public:
		UUID();
		UUID(uint64_t uuid);

		const uint64_t& AsUint64_t() const { return m_UUID; }
		std::string AsString() const { return std::to_string(m_UUID); }
		operator uint64_t() const { return m_UUID; }
	};
}
}
