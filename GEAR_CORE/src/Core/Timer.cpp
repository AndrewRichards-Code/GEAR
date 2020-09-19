#include "gear_core_common.h"
#include "Timer.h"

using namespace gear;
using namespace core;

Timer::Timer()
{
	start = std::chrono::system_clock::now();
	now = std::chrono::system_clock::now();
}

Timer::~Timer()
{
}

double Timer::GetElapsedTime()
{
	m_ElapsedTime = GetTime();
	return m_ElapsedTime;
}

double Timer::GetDeltaTime()
{
	GetElapsedTime();
	m_DeltaTime = m_ElapsedTime - m_PreviousElapsedTime;
	m_PreviousElapsedTime = m_ElapsedTime;
	return m_DeltaTime;
}

Timer::operator float()
{ 
	GetDeltaTime(); 
	return static_cast<float>(m_DeltaTime); 
}

Timer::operator double()
{ 
	GetDeltaTime();
	return m_DeltaTime; 
}

double Timer::GetTime()
{
	static bool first = true;
	if (first)
	{
		start = std::chrono::system_clock::now();
		first = false;
	}

	now = std::chrono::system_clock::now();
	elapsed = now - start;
	return elapsed.count();
}
