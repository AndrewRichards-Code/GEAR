#pragma once
#include "../Common.h"

namespace ARM
{
	class Vec3;
	class Mat3
	{
	public:
		float a, b, c, d, e, f, g, h, i;
		
		Mat3();
		Mat3(float a, float b, float c, float d, float e, float f, float g, float h, float i);
		Mat3(const Vec3& a, const Vec3& b, const Vec3& c);
		~Mat3();

		float Det();
		Vec3 VecDet();

		void Transpose();
		Mat3 Transpose(const Mat3& input);

		void Inverse();
		Mat3 Inverse(const Mat3& input);

		friend std::ostream& operator<< (std::ostream& stream, const Mat3& output)
		{
			stream << output.a << ", " << output.b << ", " << output.c << std::endl;
			stream << output.d << ", " << output.e << ", " << output.f << std::endl;
			stream << output.g << ", " << output.h << ", " << output.i;
			return stream;
		}
	};
}
