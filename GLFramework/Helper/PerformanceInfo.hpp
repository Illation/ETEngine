#pragma once

class PerformanceInfo : public Singleton<PerformanceInfo>
{
public:
	PerformanceInfo();
	virtual ~PerformanceInfo();

	unsigned int m_DrawCalls = 0;
	unsigned int m_PrevDrawCalls = 0;
private:
	friend class AbstractFramework; //should init and destroy singleton
};