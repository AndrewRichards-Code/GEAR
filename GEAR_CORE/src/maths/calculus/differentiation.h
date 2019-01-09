#pragma once
#include "../common.h"

namespace ARM
{
	double Differentiate(std::function<double(double)> function, double a, double h = 0.000001);

	template<int N>
	double PartialDifferentiate(std::function<double(std::array<double, N>)> function, const std::array<double, N>& input, unsigned int di, double h = 0.000001)
	{
		if (di < N)
		{
			std::array<double, N> values = input;
			values[di] += h;
			return (function(values) - function(input)) / h;
		}
		else
		{
			return 0.0;
		}
	}
}