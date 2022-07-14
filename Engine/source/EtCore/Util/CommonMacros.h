#pragma once

#ifdef ET_PLATFORM_WIN
#	include <windows.h>
#endif

#define ET_CAT(x, y) x##y


// for enabling or disabling features at compile time 
// avoids false compilation from missing includes of compile time switches by throwing a compiler error when the switch is not defined
// https://stackoverflow.com/questions/33759787/generating-an-error-if-checked-boolean-macro-is-not-defined
#define ET_ENABLED 1
#define ET_DISABLED 0

#define ET_CT_IS_ENABLED(x) ET_CAT(0, x)


//platform independent utility to suppress unused parameter warnings from https://stackoverflow.com/questions/4851075
// use expression as sub-expression,
// then make type of full expression int, discard result
#define ET_UNUSED(x) (void)(x)



#ifdef ET_ARCH_X32																								
#	define ET_BREAK()	__asm { int 3 }																							
#else																											
#	define ET_BREAK()	__debugbreak()		
#endif	

