#include "stdafx.h"
#include "PerformanceInfo.h"

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)


namespace et {
namespace core {


void PerformanceInfo::Update()
{
	m_PrevDrawCalls = m_DrawCalls;
	m_DrawCalls = 0;

	// on the first frame there may be no scene, and therefore no context
	// #todo: find a better way than this workaround (init default scene before first tick)
	BaseContext const* const context = ContextManager::GetInstance()->GetActiveContext();
	if (context != nullptr)
	{
		m_RegFPSTimer += context->time->DeltaTime();

		if (m_RegFPSTimer > 1.f)
		{
			m_RegFPSTimer = 0.f;
			m_RegularFPS = (int32)context->time->FPS();
		}

		m_FrameMS = context->time->DeltaTime() *1000;
	}
}

void PerformanceInfo::StartFrameTimer()
{
	m_FrameMSStart = ContextManager::GetInstance()->GetActiveContext()->time->GetTime();
}


} // namespace core
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
