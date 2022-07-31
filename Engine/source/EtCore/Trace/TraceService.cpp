#include "stdafx.h"
#include "TraceService.h"

#include "ConsoleTraceHandler.h"
#include "DebugOutputTraceHandler.h"


namespace et {
namespace core {


//====================================
// Trace Service :: Context Container
//====================================


//-----------------------------------
// ContextContainer::RegisterContext
//
T_TraceContext TraceService::ContextContainer::RegisterContext(std::string const& name)
{
	T_TraceContext const ret(name.c_str());

	if (m_Contexts.find(ret) != m_Contexts.cend())
	{
		ET_BREAK(); // Can't reliably use asserts because contexts will be registered before asserts are initialized
		return T_TraceContext();
	}

	m_Contexts.emplace(ret, name);
	return ret;
}

//----------------------------------
// ContextContainer::GetContextName
//
std::string const& TraceService::ContextContainer::GetContextName(T_TraceContext const hash)
{
	static std::string const s_InvalidContextName = "invalid_trace_context";

	auto const foundIt = m_Contexts.find(hash);
	if (foundIt == m_Contexts.cend())
	{
		ET_WARNING("invalid trace context name!");
		return s_InvalidContextName;
	}

	return foundIt->second;
}


//===============
// Trace Service
//===============


RefPtr<TraceService> TraceService::s_Instance = nullptr;


//--------------------------
// TraceService::Initialize
//
void TraceService::Initialize(bool const addDefaultHandlers)
{
	ET_ASSERT(!IsInitialized());
	s_Instance = Create<TraceService>(addDefaultHandlers);
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

//-----------------------------
// TraceService::IsInitialized
//
TraceService::ContextContainer& TraceService::GetContextContainer()
{
	static ContextContainer s_ContextContainer;
	return s_ContextContainer;
}

//----------------------------
// TraceService::TraceService
//
TraceService::TraceService(bool const addDefaultHandlers)
{
	AddHandler<core::ConsoleTraceHandler>();
#if ET_CT_IS_ENABLED(ET_CT_TRACE_DBG_OUT)
	AddHandler<core::DebugOutputTraceHandler>();
#endif
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
