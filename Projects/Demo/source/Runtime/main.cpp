#include "stdafx.h"
#include "MainFramework.h"

#include <iostream>

#include <EtCore/FileSystem/FileUtil.h>

#include <Common/linkerHelper.h>

#include <Runtime/_generated/compiled_package.h>


void SetDebuggingOptions();


//---------------------------------
// main
//
// Entry point into the framework. 
//
int main(int argc, char *argv[])
{
	using namespace et::demo;

	// set up environment
	//-------------------------
	SetDebuggingOptions();

	et::demo::ForceLinking(); // makes sure the linker doesn't ignore reflection only data

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

	// run the actual framework
	//--------------------------
	MainFramework* pFW = new MainFramework();
	pFW->Run();

	delete pFW;
	return 0;
}

//---------------------------------
// SetDebuggingOptions
//
// On debug builds this will tell us about memory leaks
//
void SetDebuggingOptions()
{
	//notify user if heap is corrupt
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// Enable run-time memory leak check for debug builds.
#if defined(ET_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	typedef HRESULT(__stdcall *fPtr)(const IID&, void**);

	//_CrtSetBreakAlloc(106750);
#endif
}
