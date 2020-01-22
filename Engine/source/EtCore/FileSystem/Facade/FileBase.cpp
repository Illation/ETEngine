#include "stdafx.h"

#include "FileBase.h"

#if defined(PLATFORM_Linux)
    #include "FileBaseLinux.hpp"
#elif defined(PLATFORM_Win)
    #include "FileBaseWin.hpp"
#else
    #include "FileBaseStub.hpp"
#endif
