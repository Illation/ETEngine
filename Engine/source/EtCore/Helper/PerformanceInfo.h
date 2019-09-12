#pragma once
#include "Singleton.h"

class PerformanceInfo : public Singleton<PerformanceInfo>
{
public:
	PerformanceInfo();
	virtual ~PerformanceInfo();

	uint32 m_DrawCalls = 0;
	uint32 m_PrevDrawCalls = 0;

	int32 GetRegularFPS() { return m_RegularFPS; }
	float GetFrameMS() { return m_FrameMS; }

private:
	friend class TickManager;

	// These two functions should only be called once per cycle
	void Update();
	void StartFrameTimer();

private:

	float m_RegFPSTimer = 10;
	int32 m_RegularFPS = 0;

	float m_FrameMS = 0;
	float m_FrameMSStart = 0;
};