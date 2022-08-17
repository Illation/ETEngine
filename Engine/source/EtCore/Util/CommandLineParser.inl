#pragma once


namespace et {
namespace core {


//===================
// CommandLineParser
//===================


//-----------------------------------
// CommandLineParser::RegisterOption
//
template <typename TDataType>
void CommandLineParser::RegisterOption(TDataType& data, std::string const& name, std::string const& description, char const shorthand /*= 0*/)
{
	RegisterOptionInternal(reinterpret_cast<uintptr_t>(&data), rttr::type::get<TDataType>(), name, shorthand, description);
}


} // namespace core
} // namespace et
