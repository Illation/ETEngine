#include "stdafx.h"
#include "Commands.h"
#include "Command.h"
#include <EtCore/FileSystem/FileUtil.h>

void DebugCopyResourceFiles()
{
#ifdef PLATFORM_Win
	if (IsDebuggerPresent())
	{
		LOG(execConsoleCommand(FileUtil::GetAbsolutePath("..\\..\\..\\build\\copyResources_windows.bat") 
			+ std::string(" ") + FileUtil::GetAbsolutePath("..\\..\\..\\source") 
			+ std::string(" ") + FileUtil::GetExecutableDir() 
			+ std::string(" x32")));
	}
#endif
}