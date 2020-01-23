#include "stdafx.h"
#include "Time.h"


namespace et {
namespace core {


Time::Time()
{
	Start();
}

Time::~Time()
{}

void Time::Start()
{
	begin = Now();
	last = begin;
}
void Time::Update()
{
	auto end = Now();
	m_DeltaTime = HRTCast<float>( Diff( last, end ) );
	last = end;
}

float Time::GetTime() const
{
	auto end = Now();
	return HRTCast<float>( Diff( begin, end ) );
}
float Time::DeltaTime() const
{
	return m_DeltaTime;
}
float Time::FPS() const
{
	return (1.f / m_DeltaTime);
}
uint64 Time::Timestamp() const
{
#ifndef PLATFORM_Linux
	return static_cast<uint64>(std::chrono::duration_cast<std::chrono::nanoseconds>(Diff(begin, Now())).count());
#endif
}
uint64 Time::SystemTimestamp() const
{
#ifndef PLATFORM_Linux
	auto end = Now();
	return static_cast<uint64>(std::chrono::duration_cast<std::chrono::nanoseconds>(end.time_since_epoch()).count());
#endif
}

//Platform abstraction
#ifdef PLATFORM_Linux

	HighResTime Time::Now()const
	{
		HighResTime ret;
		clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ret );
		return ret;
	}
	HighResDuration Time::Diff( const HighResTime &start, const HighResTime &end )const
	{
		HighResDuration ret;
		auto nsecDiff = end.tv_nsec - start.tv_nsec;
		if(nsecDiff < 0)
		{
			ret.tv_sec = end.tv_sec - start.tv_sec - 1;
			ret.tv_nsec = 1000000000 + nsecDiff;
		}
		else
		{
			ret.tv_sec = end.tv_sec - start.tv_sec;
			ret.tv_nsec = nsecDiff;
		}
		return ret;
	}

#else

	HighResTime Time::Now()const
	{
		return std::chrono::high_resolution_clock::now();
	}
	HighResDuration Time::Diff( const HighResTime &start, const HighResTime &end )const
	{
		return end - start;
	}

#endif


} // namespace core
} // namespace et
