#pragma once
#include "TraceFwd.h"


namespace et {
namespace core {


//----------------
// I_TraceHandler
//
// Implement a way for the trace service to handle trace messages - trace handlers are unique within the trace service
//
class I_TraceHandler
{
	// type info so that we can deduce the underlying type from an instance of the base class
	RTTR_ENABLE()

	// static utility
	//----------------
public:
	static std::string BuildBasicMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message);

	// construct destruct
	//--------------------
	virtual ~I_TraceHandler() = default;

	// interface
	//-----------
	virtual bool Initialize() { return true; } // return true if the handler is capable of printing trace messages
	virtual void OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message) = 0;
};


} // namespace core
} // namespace et
