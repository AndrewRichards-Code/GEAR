#include "Vec4.h"

namespace ARM
{
	//Constructs a Vec4 of 0.
	Vec4::Vec4()
		:x(0), y(0), z(0), w(0) {}

	//Constructs a Vec4 taking x, y, z, w.
	Vec4::Vec4(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w) {}

	//Constructs a Vec4 from another Vec4.
	Vec4::Vec4(const Vec4 & copy)
		: x(copy.x), y(copy.y), z(copy.z), w(copy.w) {}

	//Constructs a Vec4 from two Vec2 in the form of the first Vec2 go into x, y and the second Vec2 go into z, w.
	Vec4::Vec4(const Vec2 & a, const Vec2 & b)
		: x(a.x), y(a.y), z(b.x), w(b.y) {}

	//Constructs a Vec4 from a Vec3 and a 'w' value.
	Vec4::Vec4(const Vec3& copy, float w)
		:x(copy.x), y(copy.y), z(copy.z), w(w){}

	//Destructs the Vec4.
	Vec4::~Vec4() {}

	//Takes the dot product of the current object and another Vec4.
	float Vec4::Dot(const Vec4& other)
	{
		return (this->x * other.x) + (this->y * other.y) + (this->z * other.z) + (this->w * other.w);
	}

	//Normalise the current object.
	Vec4 Vec4::Normalise()
	{
		return *this * (1 / sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2) + pow(this->w, 2)));
	}

	//Normalise the input object and return a new Vec4.
	Vec4 Vec4::Normalise(const Vec4& other)
	{
		return other * (1 / sqrt(pow(other.x, 2) + pow(other.y, 2) + pow(other.z, 2) + pow(other.w, 2)));
	}

	//Returns the length of the Vector.
	float Vec4::Length()
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}

	//Adds two Vec4s.
	Vec4 Vec4::operator+ (const Vec4& other) const
	{
		return Vec4 (x + other.x, y + other.y, z + other.z, w + other.w);
	}

	//Subtracts two Vec4s.
	Vec4 Vec4::operator- (const Vec4& other) const
	{
		return Vec4 (x - other.x, y - other.y, z - other.z, w - other.w);
	}

	//Scales the Vec4 by the scaler a. The scaler go on the rhs of the object.
	Vec4  Vec4 ::operator* (float a) const
	{
		return Vec4 (a * x, a * y, a * z, a * w);
	}

	//Compare the Vec4 with another Vec4. If it's equal, it'll return true.
	bool Vec4 ::operator==(const Vec4 & other) const
	{
		if (x == other.x && y == other.y && z == other.z && w == other.w)
			return true;
		else
			return false;
	}

	//Compare the Vec4 with another Vec4. If it's not equal, it'll return true.
	bool Vec4 ::operator!=(const Vec4 & other) const
	{
		if (x != other.x && y != other.y && z != other.z && w != other.w)
			return true;
		else
			return false;
	}
}
