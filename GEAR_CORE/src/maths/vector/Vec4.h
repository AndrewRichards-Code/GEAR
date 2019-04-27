#pragma once
#include "../Common.h"

namespace ARM
{
	class Mat4;
	class Vec4
	{
	public:
		union
		{
			struct { float x, y, z, w; };
			struct { float r, g, b, a; };
			struct { float s, t, p, q; };
		};

		Vec4();
		Vec4(float x, float y, float z, float w);
		Vec4(const Vec4 & copy);
		Vec4(const Vec2 & a, const Vec2 & b);
		Vec4(const Vec3& copy, float w);

		~Vec4();

		float Dot(const Vec4& other);

		Vec4 Normalise();
		Vec4 Normalise(const Vec4& other);

		float Length();

		//Angles betweem vectors and normals

		//ROTATIONS
		//Rot by matrix
		Vec4 RotByMat(const Mat4& input);
		//Quaternions
		Vec4 RotQuat();
		
		Vec4 operator+ (const Vec4& other) const;
		Vec4 operator- (const Vec4& other) const;
		Vec4 operator* (float a) const;

		bool operator== (const Vec4& other) const;
		bool operator!= (const Vec4& other) const;

		friend std::ostream& operator<< (std::ostream& stream, const Vec4& output)
		{
			stream << output.x << ", " << output.y << ", " << output.z << ", " << output.w;
			return stream;
		}
	};
}
