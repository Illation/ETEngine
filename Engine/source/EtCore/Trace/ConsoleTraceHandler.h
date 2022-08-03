#pragma once
#include "TraceHandlerInterface.h"


namespace et {
namespace core {


//---------------------
// ConsoleTraceHandler
//
// Processes trace messages by sending them to a debug console
//
class ConsoleTraceHandler : public I_TraceHandler
{
	RTTR_ENABLE(I_TraceHandler)

public:
	virtual ~ConsoleTraceHandler();

	virtual bool Initialize() override;
	virtual void OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message) override;


	// Data
	///////

private:
	Ptr<std::ostream> m_OutStream;

#ifdef ET_PLATFORM_WIN
	Ptr<FILE> m_StdInPtr;
	Ptr<FILE> m_StdOutPtr;
	Ptr<FILE> m_StdErrPtr;
	HANDLE m_ConsoleHandle;
	bool m_HadConsoleWindow = false;
#endif
};


} // namespace core
} // namespace et
