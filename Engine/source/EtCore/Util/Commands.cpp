#include "stdafx.h"
#include "Commands.h"
#include "Command.h"
#include <EtCore/FileSystem/FileUtil.h>


namespace et {
namespace core {


// doesn't work anymore, keeping code as an example
void DebugCopyResourceFiles()
{
#ifdef ET_PLATFORM_WIN
	if (IsDebuggerPresent())
	{
		ET_LOG_I(ET_CTX_CORE, execConsoleCommand(FileUtil::GetAbsolutePath("..\\..\\..\\build\\copyResources_windows.bat") 
			+ std::string(" ") + FileUtil::GetAbsolutePath("..\\..\\..\\source") 
			+ std::string(" ") + FileUtil::GetExecutableDir() 
			+ std::string(" x32")).c_str());
	}
#endif
}


} // namespace core
} // namespace et
