#pragma once


//---------------------------------
// reflection
//
// utility functions using the reflection system
//
namespace reflection
{


template<typename TEnum>
std::string EnumString(TEnum const value);

template<typename TEnum>
TEnum EnumFromString(std::string const& value);


}


#include "ReflectionUtil.inl"
