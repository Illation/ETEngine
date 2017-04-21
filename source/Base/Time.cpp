#include "stdafx.hpp"
#include "Time.hpp"

Time::Time()
{
	Start();
}


Time::~Time()
{
}

void Time::Start()
{
	begin = std::chrono::high_resolution_clock::now();
	last = begin;
}
void Time::Update()
{
	auto end = std::chrono::high_resolution_clock::now();
	m_DeltaTime = ((float)(std::chrono::duration_cast<std::chrono::nanoseconds>(end - last).count()))*1e-9f;
	last = end;
}
float Time::GetTime()
{
	auto end = std::chrono::high_resolution_clock::now();
	return ((float)(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()))*1e-9f;
}
float Time::DeltaTime()
{
	return m_DeltaTime;
}
float Time::FPS()
{
	return (1.f / m_DeltaTime);
}