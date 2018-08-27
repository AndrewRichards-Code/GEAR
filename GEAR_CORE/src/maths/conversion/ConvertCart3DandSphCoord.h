#pragma once
#include <cmath>

namespace ARM
{
	//Takes in an x, y and z for the coords.
	struct CoordCart3D
	{
		double x, y, z;
		CoordCart3D(double x, double y, double z)
			:x(x), y(y), z(z) {}
	};

	//Takes in an r, theta(in radians) and phi(in radians) for the coords.
	struct CoordSph
	{
		double r, theta, phi;
		CoordSph(double r, double theta, double phi)
			:r(r), theta(theta), phi(phi) {}
	};

	//Converts cartesian coordinates to spherical coordinates.
	inline CoordSph Cart3DToSph(double x, double y, double z)
	{
		double r, theta, phi;
		r = sqrt(x * x + y * y + z * z);
		theta = acos(x / r);
		phi = atan2(y, x);
		CoordSph output(r, theta, phi);
		return output;
	}

	//Converts spheric coordinates to cartesian coordinates.
	inline CoordCart3D SphToCart3D(double r, double theta, double phi)
	{
		double x, y, z;
		x = r * sin(theta) * cos(phi);
		y = r * sin(theta) * sin(phi);
		z = r * cos(theta);
		CoordCart3D output(x, y, z);
		return output;
	}
}
