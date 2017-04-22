#pragma once

class PerformanceInfo : public Singleton<PerformanceInfo>
{
public:
	PerformanceInfo();
	virtual ~PerformanceInfo();

	unsigned int m_DrawCalls = 0;
	unsigned int m_PrevDrawCalls = 0;

	int GetRegularFPS() { return m_RegularFPS; }
	float GetFrameMS() { return m_FrameMS; }

private:
	friend class AbstractFramework; //should init and destroy singleton
	friend class AbstractScene;

	void Update();
	void StartFrameTimer();

	float m_RegFPSTimer = 10;
	int m_RegularFPS = 0;

	float m_FrameMS = 0;
	float m_FrameMSStart = 0;
};