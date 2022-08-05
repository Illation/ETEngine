#pragma once
#include <wtypes.h>


namespace et {
namespace core {

namespace platform {


//--------
// Util
//
struct Util
{
	static std::string const s_ExecutableExtension;
};


void DisplayLastError(std::string const& message);

void GetExecutablePathName(std::string& outPath);

std::vector<char*> GetCommandlineArgV();

void LaunchExecutable(std::string const& path);

std::vector<std::string> ListRunningProcesses();


} // namespace platform

} // namespace core
} // namespace et
