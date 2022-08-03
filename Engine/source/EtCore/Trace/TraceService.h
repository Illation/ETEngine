#pragma once
#include <io.h>

#include "TraceFwd.h"
#include "TraceEvents.h"
#include "TraceHandlerInterface.h"
#include "TraceContextContainer.h"

#include <EtCore/Memory/Create.h>
#include <EtCore/Memory/RefPointer.h>
#include <EtCore/Memory/Pointer.h>


namespace et {
namespace core {


//--------------
// TraceService
//
// Processes trace messages by sending them to a trace server or writing them to a file
//
class TraceService final
{
	// definitions
	//-------------
private:
	friend class RefPtr<TraceService>;
	friend class Create<TraceService>;

	static RefPtr<TraceService> s_Instance;

	typedef std::vector<UniquePtr<I_TraceHandler>> T_Handlers;

public:

	typedef std::function<void(T_TraceContext const context, E_TraceLevel const level, std::string const& msg)> T_CallbackFn;

	// singleton access
	//------------------
	static RefPtr<TraceService> Instance() { return s_Instance; }
	static void Initialize();
	static void Destroy();
	static bool IsInitialized();

	static TraceContextContainer& GetContextContainer();

	// construct destruct
	//--------------------
private:
	TraceService() = default;
	~TraceService() = default;

	// functionality
	//---------------
public:
	// trace
	void Trace(T_TraceContext const context, E_TraceLevel const level, bool const timestamp, std::string const& msg);

	// options
	void EnableDate(bool const enabled) { m_AddDate = enabled; }

	// handlers
	void SetupDefaultHandlers(std::string const& traceClientName, bool const canBreak);

	template <typename THandler>
	bool HasHandler() const;

	template <typename THandler>
	THandler* GetHandler();

	template <typename THandler, typename... Args>
	THandler* AddHandler(Args&&... args);

	template <typename THandler>
	void RemoveHandler();

	// listeners
	T_TraceCallbackId RegisterListener(T_TraceLevel const flags, T_TraceCallbackFn& callback);
	void UnregisterListener(T_TraceCallbackId& callbackId);

	// utility
	//---------
private:
	inline T_Handlers::iterator GetHandlerIt(rttr::type const handlerType);
	inline T_Handlers::const_iterator GetHandlerIt(rttr::type const handlerType) const;


	// Data
	///////

	std::vector<UniquePtr<I_TraceHandler>> m_Handlers;
	T_TraceEventDispatcher m_EventDispatcher;

	bool m_AddDate;
};


} // namespace core
} // namespace et


// Macro for declaring a new trace context
#define ET_DEFINE_TRACE_CTX(ctx)\
struct ctx final\
{\
	static et::core::T_TraceContext const s_Id;\
}

// should be registered once in some source file - watch out for linker optimizing this away
#define ET_REGISTER_TRACE_CTX(ctx) et::core::T_TraceContext const ctx::s_Id(et::core::TraceService::GetContextContainer().RegisterContext(#ctx))


#include "TraceService.inl"
