#pragma once

#ifdef ET_PLATFORM_WIN
#	include <windows.h>
#endif


//platform independent utility to suppress unused parameter warnings from https://stackoverflow.com/questions/4851075
// use expression as sub-expression,
// then make type of full expression int, discard result
#define UNUSED(x) (void)(x)



#ifdef ET_ARCH_X32																								
#	define ET_BREAK()	__asm { int 3 }																							
#else																											
#	define ET_BREAK()	__debugbreak()		
#endif	

