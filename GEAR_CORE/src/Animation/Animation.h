#pragma once
#include "gear_core_common.h"
#include "Core/Sequencer.h"
#include "Objects/Transform.h"

namespace gear
{
namespace animation
{
	struct NodeAnimation
	{
		enum class Type : uint32_t
		{
			TRANSLATION,
			ROTATION,
			SCALE
		};

		//First is a timepoint and Second is a transform.
		typedef std::pair<double, objects::Transform> Keyframe;
		typedef std::vector<Keyframe> Keyframes;
		
		std::string	name;
		Type		type;
		Keyframes	keyframes;
	};

	struct Animation : public core::Sequence
	{
		std::vector<NodeAnimation> nodeAnimations;
	};
}
}