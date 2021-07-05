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

		inline double const& ElapsedTime() const { return m_ElapsedTime; }
		inline double const& DeltaTime() const { return m_DeltaTime; }

	private:
		double GetTime();
		double GetElapsedTime();
		double GetDeltaTime();

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