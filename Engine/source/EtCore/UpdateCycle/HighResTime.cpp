#include "stdafx.h"
#include "HighResTime.h"


namespace et {
namespace core {


//===============
// High Res Time
//===============


//--------------------
// HighResTime::Now
//
// Return the current high resolution timestamp
//
HighResTime HighResTime::Now()
{
	HighResTime ret;

#ifdef ET_PLATFORM_LINUX
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ret.m_Impl);
#else
	ret.m_Impl = std::chrono::high_resolution_clock::now();
#endif

	return ret;
}

//-------------------------
// HighResTime::SinceEpoch
//
// Time since the system clock started
//
HighResDuration HighResTime::SinceEpoch() const
{
	HighResDuration ret;

#ifndef ET_PLATFORM_LINUX
	ret.m_Impl = m_Impl.time_since_epoch();
#endif

	return ret;
}


//===================
// High Res Duration
//===================


//-----------------------
// HighResDuration::Diff
//
// Create a duration from the difference of two timestamps
//
HighResDuration HighResDuration::Diff(HighResTime const& start, HighResTime const& end)
{
	HighResDuration ret;

#ifdef ET_PLATFORM_LINUX
	auto nsecDiff = end.m_Impl.tv_nsec - start.m_Impl.tv_nsec;
	if (nsecDiff < 0)
	{
		ret.m_Impl.tv_sec = end.m_Impl.tv_sec - start.m_Impl.tv_sec - 1;
		ret.m_Impl.tv_nsec = 1000000000 + nsecDiff;
	}
	else
	{
		ret.m_Impl.tv_sec = end.m_Impl.tv_sec - start.m_Impl.tv_sec;
		ret.m_Impl.tv_nsec = nsecDiff;
	}
#else
	ret.m_Impl = end.m_Impl - start.m_Impl;
#endif

	return ret;
}

//------------------------------
// HighResDuration::NanoSeconds
//
uint64 HighResDuration::NanoSeconds() const
{
#ifndef ET_PLATFORM_LINUX
	return static_cast<uint64>(std::chrono::duration_cast<std::chrono::nanoseconds>(m_Impl).count());
#endif
}

//---------------------
// HighResDuration:: -
//
HighResDuration operator-(HighResTime const& lhs, HighResTime const& rhs)
{
	return HighResDuration::Diff(rhs, lhs);
}


} // namespace core
} // namespace et

