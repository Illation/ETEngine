#pragma once
#include <rttr/type>


namespace et {
namespace core {


// Inline functions
//////////////////////


namespace reflection {


//---------------------------------
// EnumString
//
template<typename TEnum>
std::string EnumString(TEnum const value)
{
	rttr::enumeration const enumType = rttr::type::get<TEnum>().get_enumeration();

	rttr::string_view const name = enumType.value_to_name(value);
	return name.to_string();
}

//---------------------------------
// EnumFromString
//
template<typename TEnum>
TEnum EnumFromString(std::string const& value)
{
	rttr::enumeration const enumType = rttr::type::get<TEnum>().get_enumeration();

	rttr::variant const var = enumType.name_to_value(value.c_str());
	return var.get_value<TEnum>();
}


} // namespace reflection

} // namespace core
} // namespace et
