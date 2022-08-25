#include "stdafx.h"

#include <EtCore/Platform/PlatformUtil.h>
#include <EtCore/FileSystem/FileUtil.h>

#include <EtGUI/linkerHelper.h>

#include <EtTraceServer/TraceServer.h>
#include <EtTraceServer/_generated/compiled_package.h>

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
	et::gui::ForceLinking();

	// pass compiled data into core libraries so that core systems have access to it
	et::core::FileUtil::SetCompiledData(GetCompiledData_compiled_package());

	// working dir
	if (argc > 0)
	{
		// all engine files are seen as relative to the executable path by default, so it needs to be set
		et::core::FileUtil::SetExecutablePath(argv[0]);
	}
	else
	{
		std::cerr << "main > Couldn't get extract working directory from arguments, exiting!" << std::endl;
		return 1;
	}

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