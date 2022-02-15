#ifndef TYPES_CPP_HLSL_H
#define TYPES_CPP_HLSL_H

//CPP
#ifdef __cplusplus
#include "MARS/src/mars.h"

#define GEAR_FLOAT		float
#define GEAR_FLOAT2		mars::float2
#define GEAR_FLOAT3		mars::float3
#define GEAR_FLOAT4		mars::float4
#define GEAR_FLOAT2X2	mars::float2x2
#define GEAR_FLOAT3X3	mars::float3x3
#define GEAR_FLOAT4X4	mars::float4x4

#define GEAR_DOUBLE		double
#define GEAR_DOUBLE2	mars::double2
#define GEAR_DOUBLE3	mars::double3
#define GEAR_DOUBLE4	mars::double4
#define GEAR_DOUBLE2X2	mars::double2x2
#define GEAR_DOUBLE3X3	mars::double3x3
#define GEAR_DOUBLE4X4	mars::double4x4

#define GEAR_INT		int32_t
#define GEAR_INT2		mars::int2
#define GEAR_INT3		mars::int3
#define GEAR_INT4		mars::int4
#define GEAR_INT2X2		mars::int2x2
#define GEAR_INT3X3		mars::int3x3
#define GEAR_INT4X4		mars::int4x4

#define GEAR_UINT		uint32_t
#define GEAR_UINT2		mars::uint2
#define GEAR_UINT3		mars::uint3
#define GEAR_UINT4		mars::uint4
#define GEAR_UINT2X2	mars::uint2x2
#define GEAR_UINT3X3	mars::uint3x3
#define GEAR_UINT4X4	mars::uint4x4

template<typename T>
T saturate(T value)
{
	return std::clamp<T>(value, static_cast<T>(0), static_cast<T>(1));
}

//HLSL
#else

#define GEAR_FLOAT		float
#define GEAR_FLOAT2		float2
#define GEAR_FLOAT3		float3
#define GEAR_FLOAT4		float4
#define GEAR_FLOAT2X2	float2x2
#define GEAR_FLOAT3X3	float3x3
#define GEAR_FLOAT4X4	float4x4

#define GEAR_DOUBLE		double
#define GEAR_DOUBLE2	double2
#define GEAR_DOUBLE3	double3
#define GEAR_DOUBLE4	double4
#define GEAR_DOUBLE2X2	double2x2
#define GEAR_DOUBLE3X3	double3x3
#define GEAR_DOUBLE4X4	double4x4

#define GEAR_INT		int
#define GEAR_INT2		int2
#define GEAR_INT3		int3
#define GEAR_INT4		int4
#define GEAR_INT2X2		int2x2
#define GEAR_INT3X3		int3x3
#define GEAR_INT4X4		int4x4

#define GEAR_UINT		uint
#define GEAR_UINT2		uint2
#define GEAR_UINT3		uint3
#define GEAR_UINT4		uint4
#define GEAR_UINT2X2	uint2x2
#define GEAR_UINT3X3	uint3x3
#define GEAR_UINT4X4	uint4x4

float2x2 inverse(float2x2 value)
{
	float det = determinant(value);
	if (det == 0.0f)
		return value;

	float a = value[0][0];
	float b = value[0][1];
	float c = value[1][0];
	float d = value[1][1];

	return float2x2(
		+d / det,
		-b / det,
		-c / det,
		+a / det);
}

float3x3 inverse(float3x3 value)
{
	float det = determinant(value);
	if (det == 0.0f)
		return value;

	float a = value[0][0];
	float b = value[0][1];
	float c = value[0][2];
	float d = value[1][0];
	float e = value[1][1];
	float f = value[1][2];
	float g = value[2][0];
	float h = value[2][1];
	float i = value[2][2];

	return float3x3
	(
		+determinant(float2x2(e, f, h, i)) / det,
		-determinant(float2x2(b, c, h, i)) / det,
		+determinant(float2x2(b, c, e, f)) / det,
		-determinant(float2x2(d, f, g, i)) / det,
		+determinant(float2x2(a, c, g, i)) / det,
		-determinant(float2x2(a, c, d, f)) / det,
		+determinant(float2x2(d, e, g, h)) / det,
		-determinant(float2x2(a, b, g, h)) / det,
		+determinant(float2x2(a, b, d, e)) / det
	);
}

float4x4 inverse(float4x4 value)
{
	float det = determinant(value);
	if (det == 0.0f)
		return value;

	float a = value[0][0];
	float b = value[0][1];
	float c = value[0][2];
	float d = value[0][3];

	float e = value[1][0];
	float f = value[1][1];
	float g = value[1][2];
	float h = value[1][3];

	float i = value[2][0];
	float j = value[2][1];
	float k = value[2][2];
	float l = value[2][3];

	float m = value[3][0];
	float n = value[3][1];
	float o = value[3][2];
	float p = value[3][3];

	return float4x4
	(
		+determinant(float3x3(f, g, h, j, k, l, n, o, p)) / det,
		-determinant(float3x3(b, c, d, j, k, l, n, o, p)) / det,
		+determinant(float3x3(b, c, d, f, g, h, n, o, p)) / det,
		-determinant(float3x3(b, c, d, f, g, h, j, k, l)) / det,

		-determinant(float3x3(e, g, h, i, k, l, m, o, p)) / det,
		+determinant(float3x3(a, c, d, i, k, l, m, o, p)) / det,
		-determinant(float3x3(a, c, d, e, g, h, m, o, p)) / det,
		+determinant(float3x3(a, c, d, e, g, h, i, k, l)) / det,

		+determinant(float3x3(e, f, h, i, j, l, m, n, p)) / det,
		-determinant(float3x3(a, b, d, i, j, l, m, n, p)) / det,
		+determinant(float3x3(a, b, d, e, f, h, m, n, p)) / det,
		-determinant(float3x3(a, b, d, e, f, h, i, j, l)) / det,

		-determinant(float3x3(e, f, g, i, j, k, m, n, o)) / det,
		+determinant(float3x3(a, b, c, i, j, k, m, n, o)) / det,
		-determinant(float3x3(a, b, c, e, f, g, m, n, o)) / det,
		+determinant(float3x3(a, b, c, e, f, g, i, j, k)) / det
	);
}

#endif

#endif // TYPES_CPP_HLSL_H
