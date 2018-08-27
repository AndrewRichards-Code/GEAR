#pragma once
#include <cmath>

namespace ARM
{
	//Takes in an x and y for the coords.
	struct CoordCart2D
	{
		double x, y;
		CoordCart2D(double x, double y)
			:x(x), y(y) {}
	};
	
	//Takes in an r and theta(in radians) for the coords.
	struct CoordPolar
	{
		double r, theta;
		CoordPolar(double r, double theta)
			:r(r), theta(theta) {}
	};
	
	//Converts cartesian coordinates to polar coordinates.
	inline CoordPolar Cart2DToPolar(double x, double y)
	{
		double r, theta;
		r = sqrt(x * x + y * y);
		theta = atan2(y, x);
		CoordPolar output(r, theta);
		return output;
	}
	
	//Converts spheric coordinates to cartesian coordinates.
	inline CoordCart2D PolarToCart2D(double r, double theta)
	{
		double x, y;
		x = r * cos(theta);
		y = r * sin(theta);
		CoordCart2D output(x, y);
		return output;
	}
}