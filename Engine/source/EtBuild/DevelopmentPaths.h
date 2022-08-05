#pragma once
#include <string>
#include <vector>

#include <EtCore/Platform/AtomicTypes.h>


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
	static std::vector<std::string> const s_ConfigurationSuffixes;

	static std::string const s_EngineDirectory;

	static std::string const s_TraceServerDirectory;
	static std::string const s_TraceServerName;
};


} // namespace build
} // namespace et

#endif // ndef ET_SHIPPING
