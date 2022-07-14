#include "stdafx.h"
#include "Assert.h"


#if ET_CT_ASSERT

namespace et {
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
		core::Logger::Log("[ASSERT] " + caller + std::string(" > ") + msg, core::LogLevel::Warning, true);
		return true;
	}

	return false;
}


} // namespace detail
} // namespace et


#endif // ET_CT_ASSERT
