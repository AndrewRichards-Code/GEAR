#pragma once
#include "../Common.h"

namespace ARM
{
	class Vec3;
	class Vec4;
	class Mat4
	{
	public:
		float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
		
		Mat4();
		Mat4(float a, float b, float c, float d, float e, float f, float g, float h,
			float i, float j, float k, float l, float m, float n, float o, float p);
		Mat4(const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d);
		Mat4(float diagonal);
		~Mat4();

		float Det();
		Vec4 VecDet();

		void Transpose();
		Mat4 Transpose(const Mat4& input);

		void Inverse();
		Mat4 Inverse(const Mat4& input);

		static Mat4 Identity(); //test
		static Mat4 Orthographic(float left, float right, float bottom, float top, float near, float far); //test
		static Mat4 Perspective(double fov, float aspectRatio, float near, float far); //test
		
		static Mat4 Translation(const Vec3& translation); //test
		static Mat4 Rotation(double angle, const Vec3& axis); //test
		static Mat4 Scale(const Vec3& scale); //test

		friend std::ostream& operator<< (std::ostream& stream, const Mat4 & output)
		{
			stream << output.a << ", " << output.b << ", " << output.c << ", " << output.d << std::endl;
			stream << output.e << ", " << output.f << ", " << output.g << ", " << output.h << std::endl;
			stream << output.i << ", " << output.j << ", " << output.k << ", " << output.l << std::endl;
			stream << output.m << ", " << output.n << ", " << output.o << ", " << output.p;
			return stream;
		}
	};
}
