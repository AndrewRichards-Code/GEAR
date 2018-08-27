#pragma once
#include "../Common.h"

namespace ARM
{
	class Mat3;
	class Vec3
	{
	public:
		union
		{
			struct { float x, y, z; };
			struct { float r, g, b; };
			struct { float s, t, p; };
		};

		Vec3();
		Vec3(float x, float y, float z);
		Vec3(const Vec3 & copy);
		Vec3(const CoordCart3D& other);
		Vec3(const CoordSph& other);

		~Vec3();

		float Dot(const Vec3& other);
		Vec3 Cross(const Vec3& other);

		Vec3 Normalise();
		Vec3 Normalise(const Vec3& other);


		//Angles betweem vectors and normals

		//ROTATIONS
		//Rotation matrix
		Vec3 RotByMat(const Mat3& input);
		//Euler axis and angle (rotation vector)
		Vec3 RotVec(const Vec3& axis, float theta);
		//Euler rotations
		Vec3 RotEul(float x_theta, float y_theta, float z_theta);
		//Quaternions
		Vec3 RotQuat();

		//Equalivence operator==
		Vec3 operator+ (const Vec3& other) const;
		Vec3 operator- (const Vec3& other) const;
		Vec3 operator* (float a) const;

		bool operator== (const Vec3& other) const;
		bool operator!= (const Vec3& other) const;

		friend std::ostream& operator<< (std::ostream& stream, const Vec3& output)
		{
			stream << output.x << ", " << output.y << ", " << output.z;
			return stream;
		}
	};
}