#pragma once
#include "../Common.h"

namespace ARM
{
	class Vec3;
	class Vec4;
	class Mat4;
	class Quat
	{
	public:
		float s, i, j, k; 
	
		Quat();
		Quat(float s, float i, float j, float k);
		Quat(float angle, const Vec3& axis);
		Quat(const Vec4& sijk);
		~Quat();

		Quat Conjugate();
		static Quat Conjugate(const Quat& other);

		Quat Normalise();
		static Quat Normalise(const Quat& input);

		static Vec3 ToVec3(const Quat & other);
		Mat4 ToMat4();
		static Mat4 ToMat4(const Quat& input);

		Quat operator+ (const Quat& other) const;
		Quat operator- (const Quat& other) const;
		Quat operator* (const Quat& other) const;
		Quat operator* (const Vec3& other) const;

		bool operator== (const Quat& other) const;
		bool operator!= (const Quat& other) const;

		friend std::ostream& operator<< (std::ostream& stream, const Quat& output)
		{
			stream << output.s << ", " << output.i << "i, " << output.j << "j, " << output.k << "k" << std::endl;
			
			return stream;
		}
	};
}
