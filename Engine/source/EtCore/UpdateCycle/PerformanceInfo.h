#pragma once
#include <EtCore/Util/DebugUtilFwd.h>

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)

#include <EtCore/Util/Singleton.h>


namespace et {
namespace core {


class PerformanceInfo final : public Singleton<PerformanceInfo>
{
public:
	PerformanceInfo() = default;

	uint32 m_DrawCalls = 0;
	uint32 m_PrevDrawCalls = 0;

	int32 GetRegularFPS() const { return m_RegularFPS; }
	float GetFrameMS() const { return m_FrameMS; }

	// These two functions should only be called once per cycle
	void Update();
	void StartFrameTimer();

private:

	float m_RegFPSTimer = 10.f;
	int32 m_RegularFPS = 0;

	float m_FrameMS = 0;
	float m_FrameMSStart = 0;
};


} // namespace core
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
