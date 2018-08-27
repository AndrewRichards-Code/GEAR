#pragma once
#include "Common.h"
#include "MatVecMultiplication.h"

namespace ARM
{
	//Multiples, or creates the composition of, a Mat2 input by a Mat2 transform.
	inline Mat2 Mat2Mat2Multi(const Mat2& transform, const Mat2& input)
	{
		Vec2 input_i(input.a, input.c);
		Vec2 input_j(input.b, input.d);
		Vec2 output_i = transform * input_i;
		Vec2 output_j = transform * input_j;
		Mat2 output(output_i, output_j);
		output.Transpose();
		return output;
	}

	//Operator Overload * , using the Mat2Mat2Multi().
	//Multiples, or creates the composition of, a Mat2 input by a Mat2 transform.
	inline Mat2 operator* (const Mat2& transform, const Mat2& input)
	{
		return Mat2Mat2Multi(transform, input);
	}

	//Multiples, or creates the composition of, a Mat3 input by a Mat3 transform.
	inline Mat3 Mat3Mat3Multi(const Mat3& transform, const Mat3& input)
	{
		Vec3 input_i(input.a, input.d, input.g);
		Vec3 input_j(input.b, input.e, input.h);
		Vec3 input_k(input.c, input.f, input.i);
		Vec3 output_i = transform * input_i;
		Vec3 output_j = transform * input_j;
		Vec3 output_k = transform * input_k;
		Mat3 output(output_i, output_j, output_k);
		output.Transpose();
		return output;
	}

	//Operator Overload * , using the Mat3Mat3Multi().
	//Multiples, or creates the composition of, a Mat3 input by a Mat3 transform.
	inline Mat3 operator* (const Mat3& transform, const Mat3& input)
	{
		return Mat3Mat3Multi(transform, input);
	}
	
	//Multiples, or creates the composition of, a Mat4 input by a Mat4 transform.
	inline Mat4 Mat4Mat4Multi(const Mat4& transform, const Mat4& input)
	{
		Vec4 input_i(input.a, input.e, input.i, input.m);
		Vec4 input_j(input.b, input.f, input.j, input.n);
		Vec4 input_k(input.c, input.g, input.k, input.o);
		Vec4 input_l(input.d, input.h, input.l, input.p);
		Vec4 output_i = transform * input_i;
		Vec4 output_j = transform * input_j;
		Vec4 output_k = transform * input_k;
		Vec4 output_l = transform * input_l;
		Mat4 output(output_i, output_j, output_k, output_l);
		output.Transpose();
		return output;
	}

	//Operator Overload * , using the Mat4Mat4Multi().
	//Multiples, or creates the composition of, a Mat4 input by a Mat4 transform.
	inline Mat4 operator* (const Mat4& transform, const Mat4& input)
	{
		return Mat4Mat4Multi(transform, input);
	}
}