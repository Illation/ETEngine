#include "stdafx.h"

#include "../Entry.h"

#if defined(ET_PLATFORM_LINUX)
#include "DirectoryBaseLinux.hpp"
#elif defined(ET_PLATFORM_WIN)
#include "DirectoryBaseWindows.hpp"
#endif
