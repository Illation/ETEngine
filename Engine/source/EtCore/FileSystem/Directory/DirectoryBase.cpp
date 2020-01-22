#include "stdafx.h"

#include "../Entry.h"

#if defined(PLATFORM_Linux)
#include "DirectoryBaseLinux.hpp"
#elif defined(PLATFORM_Win)
#include "DirectoryBaseWindows.hpp"
#endif
