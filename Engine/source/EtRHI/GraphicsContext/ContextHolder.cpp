#include "stdafx.h"
#include "ContextHolder.h"


namespace et {
namespace rhi {


//================
// Context Holder
//================


// static stuff
//--------------

//-------------------------
// ContextHolder::Instance
//
// Global singleton access
//
ContextHolder& ContextHolder::Instance()
{
	static ContextHolder instance;
	return instance;
}

//----------------------------------
// ContextHolder::GetRenderDevice
//
// Quality of life access for main rhi context
//
I_RenderDevice* ContextHolder::GetRenderDevice()
{
	return ContextHolder::Instance().GetMainRenderContext().GetRenderDevice();
}


// functionality
//---------------

//----------------------------------------
// ContextHolder::CreateMainRenderContext
//
void ContextHolder::CreateMainRenderContext(Ptr<RenderWindow> const window)
{
	RenderDeviceParams params;
	params.m_VersionMajor = 4;
	params.m_VersionMinor = 5;

	params.m_IsForwardCompatible = true;
	params.m_UseES = false;

	params.m_DoubleBuffer = true;
	params.m_DepthBuffer = true;
	params.m_StencilBuffer = false;

#if ET_CT_IS_ENABLED(ET_CT_RHI_DEBUG)
	params.m_UseDebugInfo = true;
#else
	params.m_UseDebugInfo = false;
#endif

	m_MainRenderContext = GraphicsContext(window, window->CreateRenderDevice(params));
	window->RegisterListener(ToPtr(this));
}

//----------------------------------
// ContextHolder::OnWindowDestroyed
//
void ContextHolder::OnWindowDestroyed(RenderWindow const* const window)
{
	ET_ASSERT(window == m_MainRenderContext.GetSourceWindow());
	m_MainRenderContext = GraphicsContext();
}


} // namespace rhi
} // namespace et
