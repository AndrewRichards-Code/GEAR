#include "Quat.h"

namespace ARM
{
	//Constructs a Quat of 0.
	Quat::Quat()
		:s(0), i(0), j(0), k(0) {}
	
	//Constructs a Quat taking s, i, j, k.
	Quat::Quat(float s, float i, float j, float k)
		:s(s), i(i), j(j), k(k)	{}
	
	//Constructs a Quat taking s, and a Vec3.
	Quat::Quat(float s, const Vec3& ijk)
		:s(s), i(ijk.x), j(ijk.y), k(ijk.z)	{}
	
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
		double length = sqrt(s * s + i * i + j * j + k * k);
		s /= static_cast<float>(length);
		i /= static_cast<float>(length);
		j /= static_cast<float>(length);
		k /= static_cast<float>(length);
		return *this;
	}

	//Nornalises the input object.
	Quat Quat::Normalise(const Quat& input)
	{
		double length = sqrt(input.s * input.s + input.i * input.i + input.j * input.j + input.k * input.k);
		Quat output (
			(const_cast<float&>(input.s) /= static_cast<float>(length)),
			(const_cast<float&>(input.i) /= static_cast<float>(length)),
			(const_cast<float&>(input.j) /= static_cast<float>(length)),
			(const_cast<float&>(input.k) /= static_cast<float>(length))
		);
		return output;
	}

	//Converts the current object to a new Mat4.
	Mat4 Quat::QuatToMat4()
	{
		return Mat4(+s, -i, -j, -k,
					+i, +s, -k, +j,
					+j, +k, +s, -i,
					+k, -j, +i, +s);
	}

	//Converts the input object to a new Mat4.
	Mat4 Quat::QuatToMat4(const Quat & input)
	{
		return Mat4(+input.s, -input.i, -input.j, -input.k,
					+input.i, +input.s, -input.k, +input.j,
					+input.j, +input.k, +input.s, -input.i,
					+input.k, -input.j, +input.i, +input.s);
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