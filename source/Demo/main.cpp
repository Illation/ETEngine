#include "stdafx.hpp"
#include "MainFramework.hpp"
#include <Engine/FileSystem/FileUtil.h>
#include <iostream>

void SetDebuggingOptions();

int main(int argc, char *argv[])
{
	UNUSED( argc );
	UNUSED( argv );

	SetDebuggingOptions();

	// working dir
	if (argc > 0)
	{
		FileUtil::SetExecutablePath(argv[0]);
	}
	else
	{
		std::cerr << "main > Couldn't get extract working directory from arguments, exiting!" << std::endl;
		return 1;
	}

	MainFramework* pFW = new MainFramework();
	pFW->Run();
	delete pFW;
	return 0;
}

void SetDebuggingOptions()
{
	//notify user if heap is corrupt
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// Enable run-time memory leak check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	typedef HRESULT(__stdcall *fPtr)(const IID&, void**);

	//_CrtSetBreakAlloc( 26783 );
#endif
}