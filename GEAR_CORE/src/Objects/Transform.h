#pragma once
#include "MARS/MARS/src/mars.h"

namespace gear
{
namespace objects
{
	struct GEAR_API Transform
	{
		mars::Vec3 translation;
		mars::Quat orientation;
		mars::Vec3 scale;

		Transform()
			:translation(0.0f, 0.0f, 0.0f), orientation(1.0, 0.0, 0.0, 0.0), scale(1.0f, 1.0f, 1.0f) 
		{};
	};

	inline mars::Mat4 TransformToMat4(const Transform& transform)
	{
		return mars::Mat4::Translation(transform.translation) 
			* mars::Quat::ToMat4(transform.orientation)
			* mars::Mat4::Scale(transform.scale);
	}
	
	inline Transform Mat4ToTransform(const mars::Mat4 matrix)
	{
		Transform result;

		//Translation
		result.translation = mars::Vec3(matrix.d, matrix.h, matrix.l);

		//Scale
		float scale_x = mars::Vec3(matrix.a, matrix.e, matrix.i).Length();
		float scale_y = mars::Vec3(matrix.b, matrix.f, matrix.j).Length();
		float scale_z = mars::Vec3(matrix.c, matrix.g, matrix.k).Length();
		result.scale = mars::Vec3(scale_x, scale_y, scale_z);

		//Orientation
		mars::Mat4 rotation(
			matrix.a / scale_x, matrix.b / scale_y, matrix.c / scale_z, 0.0f,
			matrix.e / scale_x, matrix.f / scale_y, matrix.g / scale_z, 0.0f,
			matrix.i / scale_x, matrix.j / scale_y, matrix.k / scale_z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		result.orientation = mars::Quat::FromRotationMat4(rotation);

		return result;
	}
}
}