#pragma once

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
	friend class AbstractFramework; //should init and destroy singleton
	friend class AbstractScene;
	friend class RenderPipeline;

	void Update();
	void StartFrameTimer();

	float m_RegFPSTimer = 10;
	int32 m_RegularFPS = 0;

	float m_FrameMS = 0;
	float m_FrameMSStart = 0;
};