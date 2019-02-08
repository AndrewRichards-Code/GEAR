#include "Quat.h"

namespace ARM
{
	//Constructs a Quat of 0.
	Quat::Quat()
		:s(0), i(0), j(0), k(0) {}
	
	//Constructs a Quat taking s, i, j, k.
	Quat::Quat(float s, float i, float j, float k)
		:s(s), i(i), j(j), k(k)	{}
	
	//Constructs a Quat taking angle, and an axis.
	Quat::Quat(float angle, const Vec3& axis)
	{
		Vec3 scaledAxis = axis * sinf(angle / 2.0f);
		s = cosf(angle / 2.0f);
		i = scaledAxis.x;
		j = scaledAxis.y;
		k = scaledAxis.z;
		Normalise();
	}
	//Constructs a Quat taking Vec4.
	Quat::Quat(const Vec4& sijk)
		: s(sijk.x), i(sijk.y), j(sijk.z), k(sijk.w) {}

	//Destructs the Mat4.
	Quat::~Quat() {}

	//Finds the conjugate of the current object.
	Quat Quat::Conjugate() 
	{
		return Quat(this->s, -this->i, -this->j, -this->k);
	}

	//Finds the conjugate of the input object.
	Quat Quat::Conjugate(const Quat& other)
	{
		return Quat(other.s, -other.i, -other.j, -other.k);
	}

	//Normalises the current object.
	Quat Quat::Normalise()
	{
		float length = sqrt(s * s + i * i + j * j + k * k);
		s /= length;
		i /= length;
		j /= length;
		k /= length;
		return *this;
	}

	//Normalises the input object.
	Quat Quat::Normalise(const Quat& input)
	{
		float length = sqrt(input.s * input.s + input.i * input.i + input.j * input.j + input.k * input.k);
		float temp_s = input.s;
		float temp_i = input.i;
		float temp_j = input.j;
		float temp_k = input.k;
		
		Quat output (
			temp_s /= length,
			temp_i /= length,
			temp_j /= length,
			temp_k /= length
		);
		return output;
	}

	//Converts a Quat and a Vec3.
	Vec3 Quat::ToVec3(const Quat& other)
	{
		Vec3 result = Vec3(other.i, other.j, other.k);
		float theta = 2 * acosf(other.s);
		if (theta > 0)
		{
			result * (1.0f / sinf(theta / 2.0f));
		}
		return result;
	}

	//Converts the current object to a new Mat4.
	Mat4 Quat::ToMat4()
	{
		/*return Mat4(+s, -i, -j, -k,
					+i, +s, -k, +j,
					+j, +k, +s, -i,
					+k, -j, +i, +s);*/
		Normalise();
		return Mat4(
			(powf(s, 2) + powf(i, 2) - powf(j, 2) - powf(k, 2)), 2*(i*j - k*s), 2*(i*k + j*s), 0,
			2*(i*j + k*s), (powf(s, 2) - powf(i, 2) + powf(j, 2) - powf(k, 2)), 2*(j*k - i*s), 0,
			2*(i*k - j*s), 2*(j*k + i*s), (powf(s, 2) - powf(i, 2) - powf(j, 2) + powf(k, 2)), 0,
			0, 0, 0, 1);
	}

	//Converts the input object to a new Mat4.
	Mat4 Quat::ToMat4(const Quat& input)
	{
		/*return Mat4(+input.s, -input.i, -input.j, -input.k,
					+input.i, +input.s, -input.k, +input.j,
					+input.j, +input.k, +input.s, -input.i,
					+input.k, -input.j, +input.i, +input.s);*/
		Quat temp = input;
		temp.Normalise();
		return Mat4(
			(powf(temp.s, 2) + powf(temp.i, 2) - powf(temp.j, 2) - powf(temp.k, 2)), 2*(temp.i*temp.j - temp.k*temp.s), 2*(temp.i*temp.k + temp.j*temp.s), 0,
			2*(temp.i*temp.j + temp.k*temp.s), (powf(temp.s, 2) - powf(temp.i, 2) + powf(temp.j, 2) - powf(temp.k, 2)), 2*(temp.j*temp.k - temp.i*temp.s), 0,
			2*(temp.i*temp.k - temp.j*temp.s), 2*(temp.j*temp.k + temp.i*temp.s), (powf(temp.s, 2) + powf(temp.i, 2) - powf(temp.j, 2) + powf(temp.k, 2)), 0,
			0, 0, 0, 1);
	}

	//Adds two Quats.
	Quat Quat::operator+(const Quat& other) const
	{
		return Quat(s + other.s, i + other.i, j + other.j, k + other.k);
	}

	//Subtracts two Quats.
	Quat Quat::operator-(const Quat& other) const
	{
		return Quat(s - other.s, i - other.i, j - other.j, k - other.k);
	}

	//Multiples two Quats.
	Quat Quat::operator*(const Quat& other) const
	{
		return Quat(
			((s * other.s) - (i * other.i) - (j * other.j) - (k * other.k)),
			((s * other.i) + (i * other.s) + (j * other.k) - (k * other.j)),
			((s * other.j) - (i * other.k) + (j * other.s) + (k * other.i)),
			((s * other.k) + (i * other.j) - (j * other.i) + (k * other.s))
		);										   
	}

	//Multiples Quat and a Vec3.
	Quat Quat::operator*(const Vec3& other) const
	{
		return Quat(
			(- (i * other.x) - (j * other.y) - (k * other.z)),
			(+ (s * other.x) + (j * other.z) - (k * other.y)),
			(+ (s * other.y) + (k * other.x) - (i * other.z)),
			(+ (s * other.z) + (i * other.y) - (j * other.x))
		);
	}

	//Compare the Quat with another Quat. If it's equal, it'll return true.
	bool Quat::operator==(const Quat& other) const
	{
		if (s == other.s && i == other.i && j == other.j && k == other.k)
			return true;
		else
			return false;
	}

	//Compare the Quat with another Quat. If it's equal, it'll return true.
	bool Quat::operator!=(const Quat& other) const
	{
		if (s != other.s && i != other.i && j != other.j && k != other.k)
			return true;
		else
			return false;
	}

}