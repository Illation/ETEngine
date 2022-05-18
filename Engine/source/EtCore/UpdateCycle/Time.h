#pragma once

#ifdef ET_PLATFORM_LINUX
#include <time.h>
#else
#include <chrono>
#endif


namespace et {
namespace core {


#ifdef ET_PLATFORM_LINUX
	typedef timespec HighResTime;
	typedef timespec HighResDuration;
#else
	typedef std::chrono::steady_clock::time_point HighResTime;
	typedef std::chrono::duration<int64, std::nano> HighResDuration;
#endif


class Time 
{
public:
	Time();
	~Time();

	void Start();
	void Update();

	float GetTime() const;
	template<typename TDataType>
	float GetTime() const
	{
		auto end = Now();
		return HRTCast<TDataType>(Diff(begin, end));
	}

	float DeltaTime() const;
	float FPS() const;
	uint64 Timestamp() const; // milliseconds
	uint64 SystemTimestamp() const; // milliseconds

private:
	//Platform abstraction
	HighResTime Now()const;
	HighResDuration Diff( const HighResTime &start, const HighResTime &end )const;

	template<typename T>
	T HRTCast( const HighResDuration &lhs )const
	{
	#ifdef ET_PLATFORM_LINUX
		return (T)lhs.tv_sec + ((T)lhs.tv_nsec / 1000000000);
	#else
		return ((T)(std::chrono::duration_cast<std::chrono::nanoseconds>(lhs).count()))*1e-9f;
	#endif
	}

	HighResTime begin;
	HighResTime last;
	float m_DeltaTime = 0.f;
};


} // namespace core
} // namespace et
