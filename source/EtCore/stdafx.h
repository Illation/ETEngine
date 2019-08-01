#pragma once

#pragma warning( disable : 4250 ) // inherit via dominance

#define NOMINMAX

#pragma region
//C RunTime Header Files
#include <wchar.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdint>
#pragma endregion stl

#pragma region
#include <EtMath/MathInc.h>
#pragma endregion math

#pragma region
template<class T>
inline void SafeDelete(T &pObjectToDelete)
{
	if (pObjectToDelete != 0)
	{
		delete(pObjectToDelete);
		pObjectToDelete = 0;
	}
}
#pragma endregion Templates

//platform independant utility to suppress unused parameter warnings from https://stackoverflow.com/questions/4851075
// use expression as sub-expression,
// then make type of full expression int, discard result
#define UNUSED(x) (void)(x)

#pragma warning(disable : 4201) //nameless struct union - used in math library

#pragma region
#include <EtCore/Helper/AtomicTypes.h>
#include <EtCore/Helper/Context.h>
#include <EtCore/Helper/Time.h>
#include <EtCore/Helper/Logger.h>
#include <EtCore/Helper/PerformanceInfo.h>
//Working singleton Set
#define TIME ContextManager::GetInstance()->GetActiveContext()->time
#define LOG(fmt, ...) Logger::Log(fmt, __VA_ARGS__);
#define PERFORMANCE PerformanceInfo::GetInstance()

// assertions
#ifdef SHIPPING
	#define ET_ASSERT(condition, ...)
#else
#ifdef PLATFORM_Win
	#define ET_ASSERT(condition, ...) Logger::ProcessAssert(condition, __FUNCSIG__, __VA_ARGS__);
#else
	#define ET_ASSERT(condition, ...) Logger::ProcessAssert(condition, __PRETTY_FUNCTION__, __VA_ARGS__);
#endif // PLATFORM_Win
#endif // SHIPPING

#pragma endregion Macros