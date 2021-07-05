#pragma once

#include "gear_core_common.h"
#include "Timer.h"

namespace gear
{
namespace core
{
	struct Sequence 
	{
		enum class Type : uint32_t
		{
			ANIMATION,
			AUDIO
		};

		Type		sequenceType;
		double		duration;
		uint32_t	framesPerSecond;
	};

	class Sequencer
	{
	protected:
		Timer m_Timer;

	public:
		virtual ~Sequencer() = default;
		virtual void Update(const Sequence* sequences, size_t sequenceCount) = 0;
	};
}
}