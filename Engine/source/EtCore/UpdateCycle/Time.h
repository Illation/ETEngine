#pragma once
#include "HighResTime.h"


namespace et {
namespace core {


//---------------
// Time
//
// Tracks elapsed time for smooth update cycles
//
class Time final
{
	// construct destruct
	//--------------------
public:
	Time();
	~Time() = default;

	// functionality
	//---------------
	void Start();
	void Update();

	// accessors
	//-----------
	float GetTime() const;
	template<typename TDataType>
	TDataType GetTime() const;

	float DeltaTime() const { return m_DeltaTime; }
	float FPS() const;
	uint64 Timestamp() const; 


	// Data
	///////

private:
	HighResTime m_Begin;
	HighResTime m_Last;
	float m_DeltaTime = 0.f;
};


} // namespace core
} // namespace et


#include "Time.inl"
