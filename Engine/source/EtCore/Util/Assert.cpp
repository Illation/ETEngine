#include "stdafx.h"
#include "Assert.h"

#if ET_CT_IS_ENABLED(ET_CT_ASSERT)

#include <EtCore/Trace/Trace.h>

#ifdef ET_PLATFORM_WIN
#	include <EtCore/Util/WindowsUtil.h>
#endif


namespace et {

	ET_DEFINE_TRACE_CTX(ET_CTX_ASSERT);
	ET_REGISTER_TRACE_CTX(ET_CTX_ASSERT);

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


} // namespace detail
} // namespace et


#endif // ET_CT_ASSERT
