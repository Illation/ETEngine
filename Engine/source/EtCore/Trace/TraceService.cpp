#include "stdafx.h"
#include "TraceService.h"

#include "ConsoleTraceHandler.h"
#include "DebugOutputTraceHandler.h"
#include "NetworkTraceHandler.h"


namespace et {
namespace core {


//===============
// Trace Service
//===============


RefPtr<TraceService> TraceService::s_Instance = nullptr;


//--------------------------
// TraceService::Initialize
//
void TraceService::Initialize()
{
	ET_ASSERT(!IsInitialized());
	s_Instance = Create<TraceService>();
}

//---------------------------
// TraceService::GetInstance
//
void TraceService::Destroy()
{
	if (IsInitialized())
	{
		ET_ASSERT(s_Instance.GetRefCount() == 1, "Singleton destruction won't deinit the trace service");
		s_Instance = nullptr;
	}
}

//-----------------------------
// TraceService::IsInitialized
//
bool TraceService::IsInitialized()
{
	return (s_Instance != nullptr);
}

//-----------------------------------
// TraceService::GetContextContainer
//
TraceContextContainer& TraceService::GetContextContainer()
{
	static TraceContextContainer s_ContextContainer;
	return s_ContextContainer;
}

//---------------------
// TraceService::Trace
//
// For now we handle tracing in the same way the engine has before
//
void TraceService::Trace(T_TraceContext const context, E_TraceLevel const level, bool const timestamp, std::string const& msg)
{
#if ET_CT_IS_DISABLED(ET_CT_TRACE_VERBOSE)
	if (level & E_TraceLevel::TL_Verbose)
	{
		return;
	}
#endif

	// generate timestamp
	//--------------------
	std::string timestampStr;
	if (timestamp)
	{
		std::stringstream timestampStream;
		SYSTEMTIME st;
		GetSystemTime(&st);

		timestampStream << "[";
		if (m_AddDate)
		{
			timestampStream << st.wYear << "/" << st.wMonth << "/" << st.wDay << "-";
		}

		timestampStream << st.wHour << "." << st.wMinute << "." << st.wSecond << ":" << st.wMilliseconds << "]";

		timestampStr += timestampStream.str();
	}

	// handle the message
	//--------------------
	for (UniquePtr<I_TraceHandler>& handler : m_Handlers)
	{
		handler->OnTraceMessage(context, level, timestampStr, msg);
	}

	// events
	//--------
	m_EventDispatcher.Notify(static_cast<T_TraceLevel>(level), new TraceEventData(context, msg));
}

//------------------------------------
// TraceService::SetupDefaultHandlers
//
// Try to setup a network trace but otherwise fallback to a console trace
//
void TraceService::SetupDefaultHandlers(std::string const& traceClientName)
{
	AddHandler<ConsoleTraceHandler>(); // this handler is safe so it can serve to process trace messages until the network trace handler is initialized

#if ET_CT_IS_ENABLED(ET_CT_TRACE_DBG_OUT)
	AddHandler<DebugOutputTraceHandler>();
#endif

	if (AddHandler<NetworkTraceHandler>(traceClientName))
	{
		RemoveHandler<ConsoleTraceHandler>();
	}
	else
	{
		ET_WARNING("Couldn't setup a network tracehandler, falling back to console trace handler!");
	}
}

//--------------------------------
// TraceService::RegisterListener
//
T_TraceCallbackId TraceService::RegisterListener(T_TraceLevel const flags, T_TraceCallbackFn& callback)
{
	return m_EventDispatcher.Register(flags, callback);
}

//----------------------------------
// TraceService::UnregisterListener
//
void TraceService::UnregisterListener(T_TraceCallbackId& callbackId)
{
	m_EventDispatcher.Unregister(callbackId);
}


} // namespace core
} // namespace et
