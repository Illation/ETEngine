#include "stdafx.h"
#include "Assert.h"

#if ET_CT_IS_ENABLED(ET_CT_ASSERT)

#include "Trace.h"

#ifdef ET_PLATFORM_WIN
#	include <EtCore/Util/WindowsUtil.h>
#endif


namespace et {

	ET_DEFINE_TRACE_CTX(ET_CTX_ASSERT);
	ET_REGISTER_TRACE_CTX(ET_CTX_ASSERT);

	ET_DEFINE_TRACE_CTX(ET_CTX_REPORT);
	ET_REGISTER_TRACE_CTX(ET_CTX_REPORT);

namespace detail {


//========
// Assert
//========


//---------------
// ProcessAssert
//
bool ProcessAssert(bool const condition, std::string const& caller, std::string const& msg)
{
	if (!condition)
	{
		ET_TRACE_W(ET_CTX_ASSERT, "%s > %s", caller.c_str(), msg.c_str());

#ifdef ET_PLATFORM_WIN  // #todo: platform make debugger checks and message boxes platform independent
		if (!IsDebuggerPresent())
		{
			MessageBox(0, msg.c_str(), "ASSERT", 0);
		}
#endif // ET_PLATFORM_WIN

		return true;
	}

	return false;
}

//---------------
// ProcessReport
//
void ProcessReport(core::E_TraceLevel const level, std::string const& caller, std::string const& msg)
{
	ET_TRACE(ET_CTX_REPORT, level, false, "%s > %s", caller.c_str(), msg.c_str());

#ifdef ET_PLATFORM_WIN  
	if (!IsDebuggerPresent())
	{
		switch (level)
		{
		case core::E_TraceLevel::TL_Warning:
			MessageBox(0, msg.c_str(), "WARNING", 0);
			break;

		case core::E_TraceLevel::TL_Error:
			MessageBox(0, msg.c_str(), "ERROR", 0);
			break;

		case core::E_TraceLevel::TL_Fatal:
			MessageBox(0, msg.c_str(), "FATAL", 0);
			break;

		default:
			MessageBox(0, msg.c_str(), "UNSPECIFIED REPORT", 0);
			break;
		}
	}
#endif // ET_PLATFORM_WIN
}

//---------------
// FatalHandler
//
// crash out of the program before we do more damage
//
void FatalHandler(core::E_TraceLevel const level)
{
	if (level & core::E_TraceLevel::TL_Fatal)
	{
		exit(-1);
	}
}


} // namespace detail
} // namespace et


#endif // ET_CT_ASSERT
