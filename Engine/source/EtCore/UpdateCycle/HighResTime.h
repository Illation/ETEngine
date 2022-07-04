#pragma once

#ifdef ET_PLATFORM_LINUX
#	include <time.h>
#else
#	include <chrono>
#endif


namespace et {
namespace core {


struct HighResDuration;

//--------------------
// HighResTime
//
// Timestamp
//
struct HighResTime
{
	// definitions
	//-------------
private:
#ifdef ET_PLATFORM_LINUX
	typedef timespec T_HighResTime;
#else
	typedef std::chrono::steady_clock::time_point T_HighResTime;
#endif

	friend class HighResDuration;

	// static
	//--------
public:
	static HighResTime Now();

	HighResDuration SinceEpoch() const;

	// Data
	///////

private:
	T_HighResTime m_Impl;
};

//--------------------
// HighResDuration
//
// Difference between two timestamps
//
struct HighResDuration
{
	// definitions
	//-------------
private:
#ifdef ET_PLATFORM_LINUX
	typedef timespec T_HighResDuration;
#else
	typedef std::chrono::duration<int64, std::nano> T_HighResDuration;
#endif

	friend class HighResTime;

	// static
	//--------
public:
	static HighResDuration Diff(HighResTime const& start, HighResTime const& end);

	// accessors
	//-----------
	template <typename TDataType>
	TDataType Cast() const;

	uint64 NanoSeconds() const;

	// Data
	///////

private:
	T_HighResDuration m_Impl;
};

HighResDuration operator- (HighResTime const& lhs, HighResTime const& rhs);


} // namespace core
} // namespace et


#include "HighResTime.inl"
