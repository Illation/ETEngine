#include "stdafx.hpp"
#include "Commands.h"
#include "Command.h"

void DebugCopyResourceFiles()
{
#ifdef PLATFORM_Win
	if (IsDebuggerPresent())
	{
		LOG(execConsoleCommand("..\\..\\..\\build\\copyResources_windows.bat ..\\..\\..\\source . x32"));
	}
#endif
}