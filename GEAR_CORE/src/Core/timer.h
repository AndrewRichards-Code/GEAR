#pragma once

#include "gear_core_common.h"

namespace gear
{
namespace core
{
	class Timer
	{
	public:
		Timer(double time = 0.0);
		~Timer();

		double GetElapsedTime();
		double GetDeltaTime();

		operator float();
		operator double();

	private:
		double m_DeltaTime;
		double m_PreviousElapsedTime = 0.0;
		double m_ElapsedTime = 0.0;
	};
}
}