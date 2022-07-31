#include "stdafx.h"
#include "DebugOutputTraceHandler.h"

#ifdef ET_PLATFORM_WIN
#	include <EtCore/Util/WindowsUtil.h>
#endif


namespace et {
namespace core {


//============================
// Debug Output Trace Handler
//============================


//-------------------------------------
// DebugOutputTraceHandler::Initialize
//
bool DebugOutputTraceHandler::Initialize()
{
#ifdef ET_PLATFORM_WIN
	return IsDebuggerPresent();
#else
	return false;
#endif
}

//-----------------------------------------
// DebugOutputTraceHandler::OnTraceMessage
//
void DebugOutputTraceHandler::OnTraceMessage(T_TraceContext const context, 
	E_TraceLevel const level, 
	std::string const& timestamp, 
	std::string const& message)
{
#ifdef ET_PLATFORM_WIN
	if (IsDebuggerPresent()) // in case the debugger is detached - #todo: ideally find an event based way to remove this handler at that point
	{
		OutputDebugString(I_TraceHandler::BuildBasicMessage(context, level, timestamp, message).c_str());
	}
#else
	ET_ERROR("Debug Output Trace Handler shouldn't be added on unsupported platforms");
#endif
}


} // namespace core
} // namespace et
