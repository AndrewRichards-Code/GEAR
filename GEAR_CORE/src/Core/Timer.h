#pragma once

#include "gear_core_common.h"

namespace gear 
{
namespace core 
{
	class Timer
	{
	public:
		Timer();
		~Timer();

		double GetElapsedTime();
		double GetDeltaTime();

		operator float();
		operator double();

	private:
		double GetTime();

	private:
		double m_DeltaTime;
		double m_PreviousElapsedTime = 0.0;
		double m_ElapsedTime = 0.0;
		
		std::chrono::time_point<std::chrono::system_clock> start;
		std::chrono::time_point<std::chrono::system_clock> now;
		std::chrono::duration<double> elapsed;
	};
}
}