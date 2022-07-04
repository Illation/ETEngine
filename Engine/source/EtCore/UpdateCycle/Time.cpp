#include "stdafx.h"
#include "Time.h"


namespace et {
namespace core {


//======
// Time
//======


//---------------------
// Time::c-tor
//
Time::Time()
{
	Start();
}

//---------------------
// Time::Start
//
// Begin elapsing time
//
void Time::Start()
{
	m_Begin = HighResTime::Now();
	m_Last = m_Begin;
}

//---------------------
// Time::Update
//
// Update elapsed time
//
void Time::Update()
{
	HighResTime const end = HighResTime::Now();
	m_DeltaTime = (end - m_Last).Cast<float>();
	m_Last = end;
}

//---------------------
// Time::GetTime
//
// Total elapsed time
//
float Time::GetTime() const
{
	return GetTime<float>();
}

//---------------------
// Time::FPS
//
float Time::FPS() const
{
	return (1.f / m_DeltaTime);
}

//---------------------
// Time::Timestamp
//
// uint containing the time in nanoseconds since the game started
//
uint64 Time::Timestamp() const
{
	return (m_Last - m_Begin).NanoSeconds();
}


} // namespace core
} // namespace et
