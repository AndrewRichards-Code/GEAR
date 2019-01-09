#include "integration.h"

double ARM::Integrate(std::function<double(double)> function, double a, double b, int h)
{
	double range = b - a;
	double dx = range / h;
	double result = 0;
	for (int i = 0; i < h; i++)
	{
		result += function(a + i * dx) * dx;
	}
	return result;
}