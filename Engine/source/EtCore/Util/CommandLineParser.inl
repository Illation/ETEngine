#pragma once


namespace et {
namespace core {


//===================
// CommandLineParser
//===================

//------------------------------------------
// CommandLineParser::RegisterHelper::c-tor
//
// Allows registering command lines in global scope
//
template <typename TDataType>
CommandLineParser::RegisterHelper::RegisterHelper(TDataType& data, std::string const& name, std::string const& description, char const shorthand)
{
	CommandLineParser::Instance().RegisterOption(data, name, description, shorthand);
}

//-----------------------------------
// CommandLineParser::RegisterOption
//
template <typename TDataType>
void CommandLineParser::RegisterOption(TDataType& data, std::string const& name, std::string const& description, char const shorthand)
{
	T_VariantAssignFn assignFn = [](uintptr_t const data, rttr::variant const var)
	{
		return var.convert(*reinterpret_cast<TDataType*>(data));
	};

	RegisterOptionInternal(reinterpret_cast<uintptr_t>(&data), rttr::type::get<TDataType>(), assignFn, name, shorthand, description);
}

//---------------------------------
// CommandLineParser::WasOptionSet
//
template <typename TDataType>
bool CommandLineParser::WasOptionSet(TDataType const& data) const
{
	uintptr_t const ptr = reinterpret_cast<uintptr_t>(&data);
	auto const foundIt = std::find_if(m_Options.cbegin(), m_Options.cend(), [ptr](Option const& option)
		{
			return (option.m_Data == ptr);
		});

	if (foundIt != m_Options.cend())
	{
		return foundIt->m_WasSet;
	}

	return false;
}

//------------------------------------------
// CommandLineParser::ProcessVectorInternal
//
template <uint8 TCount, typename TDataType>
bool et::core::CommandLineParser::ProcessVectorInternal(std::vector<std::string> const& args, 
	char const* const format, 
	math::vector<TCount, TDataType>& vec) const
{
	for (size_t i = 0u; i < vec.data.size(); ++i)
	{
		if (i >= args.size())
		{
			ET_LOG_W(ET_CTX_CORE, "Vector arglist size is smaller than input vector dimensions");
			return false;
		}

		if (sscanf(args[i].c_str(), format, &vec.data[i]) != 1)
		{
			return false;
		}
	}

	return true;
}


} // namespace core
} // namespace et
