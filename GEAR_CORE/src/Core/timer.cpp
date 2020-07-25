#include "gear_core_common.h"
#include "Timer.h"

using namespace gear;
using namespace core;

Timer::Timer(double time)
	:m_DeltaTime(time) 
{
}

Timer::~Timer()
{
}

double Timer::GetElapsedTime()
{
	m_ElapsedTime = glfwGetTime();
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