#include "stdafx.h"
#include "Time.h"

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
float Time::GetTime()
{
	auto end = Now();
	return HRTCast<float>( Diff( begin, end ) );
}
float Time::DeltaTime()
{
	return m_DeltaTime;
}
float Time::FPS()
{
	return (1.f / m_DeltaTime);
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