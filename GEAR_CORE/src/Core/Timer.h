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

		void Update();

		inline operator float() { return static_cast<float>(m_DeltaTime); };
		inline operator double() { return m_DeltaTime; };

	private:
		double GetElapsedTime();
		double GetDeltaTime();
		double GetTime();

	private:
		double m_DeltaTime;
		double m_PreviousElapsedTime = 0.0;
		double m_ElapsedTime = 0.0;
		
		std::chrono::time_point<std::chrono::system_clock> m_StartTimePoint;
		std::chrono::time_point<std::chrono::system_clock> m_NowTimePoint;
		std::chrono::duration<double> m_ElapsedDuration;
	};
}
}