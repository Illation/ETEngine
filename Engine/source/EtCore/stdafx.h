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

#include <rttr/registration>

#pragma region
#include <EtMath/MathInc.h>
#pragma endregion math

#pragma region
template<class T>
inline void SafeDelete(T &pObjectToDelete)
{
	if (pObjectToDelete != nullptr)
	{
		delete pObjectToDelete;
		pObjectToDelete = nullptr;
	}
}
#pragma endregion Templates


#pragma warning(disable : 4201) //nameless struct union - used in math library

#pragma region
#include <EtCore/Util/AtomicTypes.h>
#include <EtCore/UpdateCycle/Context.h>
#include <EtCore/UpdateCycle/Time.h>
#include <EtCore/Util/Logger.h>
#include <EtCore/Util/StringUtil.h>
#include <EtCore/Util/CommonMacros.h>
#include <EtCore/Util/Assert.h>
#include <EtCore/Trace/Trace.h>
#include <EtCore/Hashing/HashString.h>
#include <EtCore/Reflection/ReflectionUtil.h>
#include <EtCore/Containers/VectorUtil.h>

#define TIME et::core::ContextManager::GetInstance()->GetActiveContext()->time // #todo: refactor and remove macro
#define LOG(fmt, ...) et::core::Logger::Log(fmt, __VA_ARGS__)

#include <EtCore/Memory/Pointer.h>

#pragma endregion Macros