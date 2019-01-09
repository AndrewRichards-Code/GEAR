#pragma once
#include "../Common.h"

namespace ARM
{
	class Vec2;
	
	class Mat2
	{
	public:
		float a, b, c, d;
		
		Mat2();
		Mat2(float a, float b, float c, float d);
		Mat2(const Vec2& a, const Vec2& b);
		~Mat2();

		float Det();
		Vec2 VecDet();

		void Transpose();
		Mat2 Transpose(const Mat2& input);

		void Inverse();
		Mat2 Inverse(const Mat2& input);

		friend std::ostream& operator<< (std::ostream& stream, const Mat2& output)
		{
			stream << output.a << ", " << output.b << std::endl;
			stream << output.c << ", " << output.d;
			
			return stream;
		}
	};
}