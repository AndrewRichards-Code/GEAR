#pragma once
#include "../Common.h"

namespace ARM
{
	class Mat2;
	class Vec2
	{
	public:
		union
		{
			struct { float x, y; };
			struct { float r, g; };
			struct { float s, t; };
		};
		
		Vec2();
		Vec2(float x, float y);
		Vec2(const Vec2& copy);
		Vec2(const CoordCart2D& other);
		Vec2(const CoordPolar& other);

		~Vec2();

		float Dot(const Vec2& other);
		float Det(const Vec2& other);

		Vec2 Normalise();
		Vec2 Normalise(const Vec2& other);

		float Length();
		
		//Angles betweem vectors and normals

		//ROTATIONS
		//Rotation matrix
		Vec2 RotByMat(const Mat2& input);
		//Angles
		Vec2 RotDegRound(double theta);
		Vec2 RotRadRound(double theta);
		Vec2 RotDeg(double theta);
		Vec2 RotRad(double theta);
		Vec2 Rot090();
		Vec2 Rot180();
		Vec2 Rot270();
		
		//Equalivence operator==
		Vec2 operator+ (const Vec2& other) const;
		Vec2 operator- (const Vec2& other) const;
		Vec2 operator* (float a) const;

		bool operator== (const Vec2& other) const;
		bool operator!= (const Vec2& other) const;

		friend std::ostream& operator<< (std::ostream& stream, const Vec2& output)
		{
			stream << output.x << ", " << output.y;
			return stream;
		}
	};
}