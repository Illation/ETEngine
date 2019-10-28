#pragma once
#include <rttr/type>


//---------------------------------
// reflection
//
// utility functions using the reflection system
//
namespace reflection
{


template<typename TEnum>
std::string EnumString(TEnum const value);


}


#include "ReflectionUtil.inl"
