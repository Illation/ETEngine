#pragma once
#include "TraceHandlerInterface.h"


namespace et {
namespace core {


//--------------------
// MemoryTraceHandler
//
// Simply stores traces messages in a list
//  - since the initialization won't fail it can be used to temporarily store incoming messages until other handlers are ready
//
class MemoryTraceHandler : public I_TraceHandler
{
	// definitions
	//-------------
	RTTR_ENABLE(I_TraceHandler)

public:
	struct TraceMessage
	{
		TraceMessage(T_TraceContext const ctx, E_TraceLevel const lvl, std::string const& ts, std::string const& msg) 
			: m_Context(ctx), m_Level(lvl), m_Timestamp(ts), m_Message(msg) {}

		T_TraceContext const m_Context;
		E_TraceLevel const m_Level;
		std::string const m_Timestamp;
		std::string const m_Message;
	};

	// construct destruct
	//--------------------
	virtual ~MemoryTraceHandler() = default;

	// interface
	//-----------
	virtual bool Initialize() { return true; }
	virtual void OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message) override;

	// functionality
	//---------------
	void Relay(I_TraceHandler* const otherHandler) const;
	void Clear() { m_Messages.clear(); }


	// Data
	///////

private:
	std::vector<TraceMessage> m_Messages;
};


} // namespace core
} // namespace et
