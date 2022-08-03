#include "stdafx.h"
#include "MemoryTraceHandler.h"


namespace et {
namespace core {


//======================
// Memory Trace Handler
//======================


//------------------------------------
// MemoryTraceHandler::OnTraceMessage
//
void MemoryTraceHandler::OnTraceMessage(T_TraceContext const context,
	E_TraceLevel const level,
	std::string const& timestamp,
	std::string const& message)
{
	m_Messages.emplace_back(context, level, timestamp, message);
}

//---------------------------
// MemoryTraceHandler::Relay
//
void MemoryTraceHandler::Relay(I_TraceHandler* const otherHandler) const
{
	for (TraceMessage const& msg : m_Messages)
	{
		otherHandler->OnTraceMessage(msg.m_Context, msg.m_Level, msg.m_Timestamp, msg.m_Message);
	}
}


} // namespace core
} // namespace et


