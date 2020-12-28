#include "stdafx.h"

#include "FileBase.h"

#if defined(ET_PLATFORM_LINUX)
    #include "FileBaseLinux.hpp"
#elif defined(ET_PLATFORM_WIN)
    #include "FileBaseWin.hpp"
#else
    #include "FileBaseStub.hpp"
#endif
