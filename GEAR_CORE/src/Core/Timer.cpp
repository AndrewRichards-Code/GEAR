#include "gear_core_common.h"
#include "Timer.h"

using namespace gear;
using namespace core;

Timer::Timer()
{
	m_StartTimePoint = std::chrono::system_clock::now();
	m_NowTimePoint = std::chrono::system_clock::now();
}

Timer::~Timer()
{
}

void Timer::Update()
{
	GetDeltaTime();
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

double Timer::GetTime()
{
	m_NowTimePoint = std::chrono::system_clock::now();
	m_ElapsedDuration = m_NowTimePoint - m_StartTimePoint;
	return m_ElapsedDuration.count();
}
