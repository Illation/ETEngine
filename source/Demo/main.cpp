#include "stdafx.hpp"
#include "MainFramework.hpp"

void SetDebuggingOptions();

int main(int argc, char *argv[])
{
	UNUSED( argc );
	UNUSED( argv );

	SetDebuggingOptions();

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

	//_CrtSetBreakAlloc( 442 );
#endif
}