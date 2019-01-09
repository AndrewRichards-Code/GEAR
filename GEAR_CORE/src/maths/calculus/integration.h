#pragma once
#include "../common.h"

namespace ARM
{
	double Integrate(std::function<double(double)> function, double a, double b, int h = 1000000);
}