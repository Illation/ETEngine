#pragma once
#include "TraceHandlerInterface.h"


namespace et {
namespace core {


//-------------------------
// DebugOutputTraceHandler
//
// Print the trace messages to a debugger, if one is attached
//
class DebugOutputTraceHandler final : public I_TraceHandler
{
	RTTR_ENABLE(I_TraceHandler)

public:
	~DebugOutputTraceHandler() = default;

	bool Initialize() override;
	void OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message) override;
};


} // namespace core
} // namespace et
