#pragma once
#include "TraceFwd.h"

#include <EtCore/Util/GenericEventDispatcher.h>


namespace et {
namespace core {


//---------------------------
// TraceEventData
//
struct TraceEventData
{
public:
	TraceEventData(T_TraceContext const context, std::string const& message) : m_Context(context), m_Message(message) {}
	virtual ~TraceEventData() = default;

	T_TraceContext const m_Context;
	std::string const& m_Message;
};


typedef GenericEventDispatcher<T_TraceLevel, TraceEventData> T_TraceEventDispatcher;


typedef T_TraceEventDispatcher::T_CallbackId T_TraceCallbackId;
typedef T_TraceEventDispatcher::T_CallbackFn T_TraceCallbackFn;


} // namespace core
} // namespace et
