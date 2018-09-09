#pragma once

namespace ARM
{
	static const double pi = 3.14151926535;
	static const double tau = 2 * pi;

	//Coverts degrees to radians.
	inline double DegToRad(double angle)
	{
		return angle * pi / 180;
	}

	//Coverts radians to degrees.
	inline double RadToDeg(double angle)
	{
		return angle * 180 / pi;
	}
}