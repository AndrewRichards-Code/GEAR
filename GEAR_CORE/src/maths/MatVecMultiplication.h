#pragma once
#include "Common.h"

namespace ARM
{
	//Multiples a Vec2 input by a Mat2 transform.
	inline Vec2 Mat2Vec2Multi(const Vec2& input, const Mat2& transform)
	{
		float x = input.x;
		float y = input.y;
		Vec2 transform_i(transform.a, transform.c);
		Vec2 transform_j(transform.b, transform.d);
		Vec2 output(transform_i * x + transform_j * y);
		return output;
	}

	//Operator Overload * , using the Mat2Vec2Multi().
	//Multiples a Vec2 input by a Mat2 transform.
	inline Vec2 operator* (const Vec2& input, const Mat2& transform)
	{
		return Mat2Vec2Multi(input, transform);
	}
	//Operator Overload * , using the Mat2Vec2Multi().
	//Multiples a Vec2 input by a Mat2 transform.
	inline Vec2 operator* (const Mat2& transform, const Vec2& input)
	{
		return Mat2Vec2Multi(input, transform);
	}

	//Multiples a Vec3 input by a Mat3 transform.
	inline Vec3 Mat3Vec3Multi(const Vec3& input, const Mat3& transform)
	{
		float x = input.x;
		float y = input.y;
		float z = input.z;
		Vec3 transform_i(transform.a, transform.d, transform.g);
		Vec3 transform_j(transform.b, transform.e, transform.h);
		Vec3 transform_k(transform.c, transform.f, transform.i);
		Vec3 output(transform_i * x + transform_j * y + transform_k * z);
		return output;
	}

	//Operator Overload * , using the Mat3Vec3Multi().
	//Multiples a Vec3 input by a Mat3 transform.
	inline Vec3 operator* (const Vec3& input, const Mat3& transform)
	{
		return Mat3Vec3Multi(input, transform);
	}
	//Operator Overload * , using the Mat3Vec3Multi().
	//Multiples a Vec3 input by a Mat3 transform.
	inline Vec3 operator* (const Mat3& transform, const Vec3& input)
	{
		return Mat3Vec3Multi(input, transform);
	}

	//Multiples a Vec4 input by a Mat4 transform.
	inline Vec4 Mat4Vec4Multi(const Vec4& input, const Mat4& transform)
	{
		float x = input.x;
		float y = input.y;
		float z = input.z;
		float w = input.w;
		Vec4 transform_i(transform.a, transform.e, transform.i, transform.m);
		Vec4 transform_j(transform.b, transform.f, transform.j, transform.n);
		Vec4 transform_k(transform.c, transform.g, transform.k, transform.o);
		Vec4 transform_l(transform.d, transform.h, transform.l, transform.p);
		Vec4 output(transform_i * x + transform_j * y + transform_k * z + transform_l * w);
		return output;
	}

	//Operator Overload * , using the Mat4Vec4Multi().
	//Multiples a Vec4 input by a Mat4 transform.
	inline Vec4 operator* (const Vec4& input, const Mat4& transform)
	{
		return Mat4Vec4Multi(input, transform);
	}
	//Operator Overload * , using the Mat4Vec4Multi().
	//Multiples a Vec4 input by a Mat4 transform.
	inline Vec4 operator* (const Mat4& transform, const Vec4& input)
	{
		return Mat4Vec4Multi(input, transform);
	}
}