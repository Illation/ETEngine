#include "stdafx.h"
#include "PerformanceInfo.hpp"

PerformanceInfo::PerformanceInfo()
{
}
PerformanceInfo::~PerformanceInfo()
{
}

void PerformanceInfo::Update()
{
	m_PrevDrawCalls = m_DrawCalls;
	m_DrawCalls = 0;

	m_RegFPSTimer += TIME->DeltaTime();
	if (m_RegFPSTimer > 1.f)
	{
		m_RegFPSTimer = 0.f;
		m_RegularFPS = (int32)TIME->FPS();
	}

	m_FrameMS = (TIME->GetTime() - m_FrameMSStart)*1000;
}
void PerformanceInfo::StartFrameTimer()
{
	m_FrameMSStart = TIME->GetTime();
}