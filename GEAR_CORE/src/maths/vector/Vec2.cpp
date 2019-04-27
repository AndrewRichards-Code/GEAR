#include "Vec2.h"

namespace ARM
{
	//Constructs a Vec2 of 0.
	Vec2::Vec2()
		: x(0), y(0) {}

	//Constructs a Vec2 taking x, y, z.
	Vec2::Vec2(float x, float y)
		: x(x), y(y) {}

	//Constructs a Vec2 from another Vec2.
	Vec2::Vec2(const Vec2& copy)
		: x(copy.x), y(copy.y) {}

	//Constructs a Vec2 from the struct CoordCart2D.
	Vec2::Vec2(const CoordCart2D& other)
		: x((float)other.x), y((float)other.y) {}

	//Constructs a Vec2 from the struct CoordPolar.
	Vec2::Vec2(const CoordPolar& other)
		: x((float)(other.r * cos(other.theta))), y((float)(other.r * sin(other.theta))) {}

	//Destructs the Vec2.
	Vec2::~Vec2() {}

	//Takes the dot product of the current object and another Vec2.
	float Vec2::Dot(const Vec2& other)
	{
		return (this->x * other.x) + (this->y * other.y);
	}

	//Takes the cross product/determinant of the current object and another Vec2.
	float Vec2::Det(const Vec2& other)
	{
		return (this->x * other.y) - (this->y * other.x);
	}

	//Normalise the current object.
	Vec2 Vec2::Normalise()
	{
		return *this * (1 / sqrt(pow(this->x, 2) + pow(this->y, 2)));
	}

	//Normalise the input object and return a new Vec2.
	Vec2 Vec2::Normalise(const Vec2& other)
	{
		return other * (1 / sqrt(pow(other.x, 2) + pow(other.y, 2)));
	}

	//Returns the length of the Vector.
	float Vec2::Length()
	{
		return sqrtf(x * x + y * y);
	}

	//Rotates the Vec2 by the input angle (in degrees). 
	//Rounds output Vec2 to nearest int. Uses with Vec2<int>!
	Vec2 Vec2::RotDegRound(double theta)
	{
		float theta_rads = (float)DegToRad(theta);
		return Vec2((float)round(x * cos(theta_rads) - y * sin(theta_rads)), (float)round(x * sin(theta_rads) + y * cos(theta_rads)));
	}

	//Rotates the Vec2 by the input angle (in radinas). 
	//Rounds output Vec2 to nearest int. Uses with Vec2<int>!
	Vec2 Vec2::RotRadRound(double theta)
	{
		return Vec2((float)round(x * cos(theta) - y * sin(theta)), (float)round(x * sin(theta) + y * cos(theta)));
	}

	//Rotates the Vec2 by the input angle (in degrees). 
	Vec2 Vec2::RotDeg(double theta)
	{
		float theta_rads = (float)DegToRad(theta);
		return Vec2((float)(x * cos(theta_rads) - y * sin(theta_rads)), (float)(x * sin(theta_rads) + y * cos(theta_rads)));
	}

	//Rotates the Vec2 by the input angle (in radinas). 
	Vec2 Vec2::RotRad(double theta)
	{
		return Vec2((float)(x * cos(theta) - y * sin(theta)), (float)(x * sin(theta) + y * cos(theta)));
	}

	//Rotates the Vec2 by 90 degress.
	Vec2 Vec2::Rot090()
	{
		return Vec2(x * 0 + y * -1, x * 1 + y * 0);
	}

	//Rotates the Vec2 by 180 degress.
	Vec2 Vec2::Rot180()
	{
		return Vec2(x * -1 + y * 0, x * 0 + y * -1);
	}

	//Rotates the Vec2 by 270 degress.
	Vec2 Vec2::Rot270()
	{
		return Vec2(x * 0 + y * 1, x * -1 + y * 0);
	}

	//Adds two Vec2s.
	Vec2 Vec2::operator+ (const Vec2& other) const
	{
		return Vec2(x + other.x, y + other.y);
	}

	//Subtracts two Vec2s.
	Vec2 Vec2::operator- (const Vec2& other) const
	{
		return Vec2(x - other.x, y - other.y);
	}

	//Scales the Vec2 by the scaler a. The scaler go on the rhs of the object.
	Vec2 Vec2::operator*(float a) const
	{
		return Vec2(a * x, a * y);
	}

	//Compare the Vec2 with another Vec2. If it's equal, it'll return true.
	bool Vec2::operator==(const Vec2& other) const
	{
		if (x == other.x && y == other.y)
			return true;
		else
			return false;
	}

	//Compare the Vec2 with another Vec2. If it's not equal, it'll return true.
	bool Vec2::operator!=(const Vec2& other) const
	{
		if (x != other.x && y != other.y)
			return true;
		else
			return false;
	}
}