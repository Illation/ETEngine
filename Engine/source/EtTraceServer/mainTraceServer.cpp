#include "stdafx.h"

#include <EtCore/linkerHelper.h>
#include <EtCore/Platform/PlatformUtil.h>

#include <EtTraceServer/TraceServer.h>

#ifdef ET_PLATFORM_WIN
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#endif


//---------------------------------
// main
//
// Main function for the trace server
//
int main(int argc, char *argv[])
{
	et::core::ForceLinking();

	et::trace::TraceServer traceServer(argc, argv);
	if (traceServer.GetReturnCode() == et::trace::TraceServer::E_ReturnCode::Success)
	{
		traceServer.Run();
	}

	return static_cast<int>(traceServer.GetReturnCode());
}

#ifdef ET_PLATFORM_WIN
//---------------------------------
// WinMain
//
// On windows this will be our entry point instead - reroute to the regular main function
//
int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	std::vector<char*> argv = et::core::platform::GetCommandlineArgV();
	return main(static_cast<int>(argv.size()), &argv[0]);
}
#endif