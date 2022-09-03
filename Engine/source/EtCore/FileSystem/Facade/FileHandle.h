#pragma once

#if defined(ET_PLATFORM_LINUX)
	#include <cstdint>
    typedef int32_t FILE_HANDLE;
    #define FILE_HANDLE_INVALID (-1)
#elif defined(ET_PLATFORM_WIN)
	#include <basetsd.h>

    typedef HANDLE FILE_HANDLE;
    #define FILE_HANDLE_INVALID INVALID_HANDLE_VALUE
#endif
