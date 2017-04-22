#pragma once
#include <chrono>
class Time
{
public:
	Time();
	~Time();
	void Start();
	void Update();
	float GetTime();
	float DeltaTime();
	float FPS();

private:
	std::chrono::steady_clock::time_point begin;
	std::chrono::steady_clock::time_point last;
	float m_DeltaTime;
};

