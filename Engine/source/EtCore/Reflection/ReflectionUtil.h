#pragma once


namespace et {
namespace core {


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


enum class E_MetaData
{
	VectorType
};


}


} // namespace core
} // namespace et


#include "ReflectionUtil.inl"
