#pragma once
#include "../Common.h"

namespace ARM
{
	class Mat3;
	class Quat;
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
		static float Dot(const Vec3& a, const Vec3& b);
		
		Vec3 Cross(const Vec3& other);
		static Vec3 Cross(const Vec3& a, const Vec3& b);
		
		Vec3 Normalise();
		static Vec3 Normalise(const Vec3& other);

		float Length();


		//Angles betweem vectors and normals

		//ROTATIONS
		//Rotation matrix
		Vec3 RotByMat(const Mat3& input);
		//Euler axis and angle (rotation vector)
		Vec3 RotVec(const Vec3& axis, float theta);
		//Euler rotations
		Vec3 RotEul(float x_theta, float y_theta, float z_theta);
		//Quaternions
		Vec3 RotQuat(float theta, const Vec3& axis);
		Vec3 RotQuat(const Quat& q);

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