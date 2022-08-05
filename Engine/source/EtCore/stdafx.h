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
#include <rttr/registration>
#pragma endregion 3rdParty

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
#include <EtMath/MathInc.h>
#include <EtCore/Platform/AtomicTypes.h>
#include <EtCore/UpdateCycle/Context.h>
#include <EtCore/UpdateCycle/Time.h>
#include <EtCore/Containers/StringUtil.h>
#include <EtCore/Util/CommonMacros.h>
#include <EtCore/Trace/Assert.h>
#include <EtCore/Trace/Trace.h>
#include <EtCore/Hashing/HashString.h>
#include <EtCore/Reflection/ReflectionUtil.h>
#include <EtCore/Containers/VectorUtil.h>
#include <EtCore/Memory/Pointer.h>
#pragma endregion Includes


namespace et {
	ET_DEFINE_TRACE_CTX(ET_CTX_CORE);
}