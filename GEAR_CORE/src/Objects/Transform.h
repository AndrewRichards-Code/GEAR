#pragma once
#include "MARS/src/mars.h"

namespace gear
{
namespace objects
{
	struct GEAR_API Transform
	{
		mars::float3 translation;
		mars::Quaternion orientation;
		mars::float3 scale;

		Transform()
			:translation(0.0f, 0.0f, 0.0f), orientation(1.0, 0.0, 0.0, 0.0), scale(1.0f, 1.0f, 1.0f) 
		{};
	};

	inline mars::float4x4 TransformToMatrix4(const Transform& transform)
	{
		return mars::float4x4::Translation(transform.translation)
			* mars::Quaternion::ToRotationMatrix4<float>(transform.orientation)
			* mars::float4x4::Scale(transform.scale);
	}
	
	inline Transform Matrix4ToTransform(const mars::float4x4 matrix)
	{
		Transform result;

		//Translation
		result.translation = mars::float3(matrix.d, matrix.h, matrix.l);

		//Scale
		float scale_x = mars::float3(matrix.a, matrix.e, matrix.i).Length();
		float scale_y = mars::float3(matrix.b, matrix.f, matrix.j).Length();
		float scale_z = mars::float3(matrix.c, matrix.g, matrix.k).Length();
		result.scale = mars::float3(scale_x, scale_y, scale_z);

		//Orientation
		mars::float4x4 rotation(
			matrix.a / scale_x, matrix.b / scale_y, matrix.c / scale_z, 0.0f,
			matrix.e / scale_x, matrix.f / scale_y, matrix.g / scale_z, 0.0f,
			matrix.i / scale_x, matrix.j / scale_y, matrix.k / scale_z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		result.orientation = mars::Quaternion::FromRotationMatrix4(rotation);

		return result;
	}
}
}