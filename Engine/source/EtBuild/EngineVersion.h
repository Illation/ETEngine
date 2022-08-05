#pragma once
#include <string>

#include <EtCore/Platform/AtomicTypes.h>


namespace et {
namespace build {


//----------------------
// Version
//
// Version info extracted from the build system and VCS
//
class Version final
{
public:
	static std::string const s_Name;
};


} // namespace build
} // namespace et
