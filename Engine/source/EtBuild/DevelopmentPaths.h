#pragma once
#include <string>

#include <EtCore/Util/AtomicTypes.h>


#ifndef ET_SHIPPING

namespace et {
namespace build {


//----------------------
// DevelopmentPaths
//
// Information about directories extracted from build system
//
class DevelopmentPaths final
{
public:
	static std::string const s_EngineDirectory;

	static std::string const s_TraceServerDirectory;
	static std::string const s_TraceServerName;
};


} // namespace build
} // namespace et

#endif // ndef ET_SHIPPING
