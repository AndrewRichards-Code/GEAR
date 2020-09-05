#pragma once
#include "mars.h"

namespace gear
{
namespace objects
{
	struct Transform
	{
		mars::Vec3 translation;
		mars::Quat orientation;
		mars::Vec3 scale;
	};

	inline mars::Mat4 TransformToMat4(const Transform& transform)
	{
		return mars::Mat4::Translation(transform.translation) 
			* mars::Quat::ToMat4(transform.orientation)
			* mars::Mat4::Scale(transform.scale);
	}
}
}