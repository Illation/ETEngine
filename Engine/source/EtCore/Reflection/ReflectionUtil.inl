#pragma once


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


} // namespace reflection

