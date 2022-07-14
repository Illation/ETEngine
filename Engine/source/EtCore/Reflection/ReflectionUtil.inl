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

//---------------------------------
// EnumFromString
//
template<typename TEnum>
bool EnumFromStringSafe(std::string const& value, TEnum& outEnum)
{
	rttr::enumeration const enumType = rttr::type::get<TEnum>().get_enumeration();

	rttr::array_range<rttr::string_view> const names = enumType.get_names();
	auto const foundIt = std::find_if(names.cbegin(), names.cend(), [&value](rttr::string_view const& view)
		{
			return (view.to_string() == value);
		});

	if (foundIt != names.cend())
	{
		rttr::variant const var = enumType.name_to_value(value.c_str());
		outEnum = var.get_value<TEnum>();
		return true;
	}

	return false;
}



} // namespace reflection

} // namespace core
} // namespace et
