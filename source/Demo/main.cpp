#include "stdafx.hpp"
#include "MainFramework.hpp"
#include "..\Engine\Helper\Command.h"

void SetDebuggingOptions();
void DebugCopyResourceFiles();

int wmain(int argc, char *argv[])
{
	UNUSED( argc );
	UNUSED( argv );

#ifndef SHIPPING
	DebugCopyResourceFiles();
#endif
	SetDebuggingOptions();

	MainFramework* pFW = new MainFramework();
	pFW->Run();
	delete pFW;
	return 0;
}

void DebugCopyResourceFiles()
{
#ifdef PLATFORM_Win
	if(IsDebuggerPresent())
	{
		std::cout << execConsoleCommand("..\\..\\..\\build\\copyResources_windows.bat ..\\..\\..\\source . x32") << std::endl;
	}
#endif
}

void SetDebuggingOptions()
{
	//notify user if heap is corrupt
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// Enable run-time memory leak check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	typedef HRESULT(__stdcall *fPtr)(const IID&, void**);

	//_CrtSetBreakAlloc( 11585 );
#endif
}