#include "differentiation.h"

using namespace ARM;

double ARM::Differentiate(std::function<double(double)> function, double a, double h)
{
	return (function(a + h) - function(a)) / h;
}