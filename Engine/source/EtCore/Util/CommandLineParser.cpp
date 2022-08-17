#include "stdafx.h"
#include "CommandLineParser.h"

#include <EtCore/Reflection/TypeInfoRegistry.h>


namespace et {
namespace core {


//===================
// CommandLineParser
//===================


//-----------------------------
// CommandLineParser::Instance
//
CommandLineParser& CommandLineParser::Instance()
{
	static CommandLineParser s_Parser;
	return s_Parser;
}

//----------------------------
// CommandLineParser::Process
//
void CommandLineParser::Process(int32 const argc, char* const argv[])
{
	Option* currentOption = nullptr;
	for (int32 idx = 1; idx < argc; ++idx)
	{
		std::string const arg = argv[idx];

		if (currentOption == nullptr)
		{
			IdentifyOption(arg, currentOption);
		}
		else
		{
			ProcessOption(*currentOption, arg);
			currentOption = nullptr;
		}
	}
}

//---------------------------------
// CommandLineParser::WasOptionSet
//
bool CommandLineParser::WasOptionSet(HashString const optionId) const
{
	auto const foundIt = std::find_if(m_Options.cbegin(), m_Options.cend(), [optionId](Option const& option)
		{
			return (option.m_Id == optionId);
		});

	if (foundIt != m_Options.cend())
	{
		return foundIt->m_WasSet;
	}

	return false;
}

//-------------------------------------------
// CommandLineParser::RegisterOptionInternal
//
void CommandLineParser::RegisterOptionInternal(uintptr_t const data, 
	rttr::type const type, 
	std::string const& name, 
	char const shorthand,
	std::string const& desc)
{
	HashString const optionId(name.c_str());

	auto const foundIt = std::find_if(m_Options.cbegin(), m_Options.cend(), [optionId](Option const& option)
		{
			return (option.m_Id == optionId);
		});

	if (foundIt != m_Options.cend())
	{
		ET_WARNING("Option '%s' already set", name.c_str());
		return;
	}

	m_Options.emplace_back(type);
	Option& option = m_Options.back();

	option.m_Id = optionId;
	option.m_ShortHand = shorthand;

	option.m_Data = data;

	option.m_Description = desc;
}

//-----------------------------------
// CommandLineParser::IdentifyOption
//
void CommandLineParser::IdentifyOption(std::string const& arg, Option*& currentOption)
{
	if (arg.size() > 1u && arg[0] == '-')
	{
		if (arg.size() > 2u && arg[1] == '-')
		{
			HashString const optionId(arg.substr(2u).c_str());
			auto const foundIt = std::find_if(m_Options.begin(), m_Options.end(), [optionId](Option const& option)
				{
					return (option.m_Id == optionId);
				});

			if (foundIt == m_Options.cend())
			{
				ET_WARNING("Option '%s' not found", optionId.ToStringDbg());
				return;
			}

			currentOption = &(*foundIt);
		}
		else
		{
			if (arg.size() > 2u)
			{
				ET_WARNING("Expected a shorthand but the argument was too long: %s", arg.c_str());
				return;
			}

			char const shorthand = arg[1];
			auto const foundIt = std::find_if(m_Options.begin(), m_Options.end(), [shorthand](Option const& option)
				{
					return (option.m_ShortHand == shorthand);
				});

			if (foundIt == m_Options.cend())
			{
				ET_WARNING("Option with shorthand '%c' not found", shorthand);
				return;
			}

			currentOption = &(*foundIt);
		}
	}
	else
	{
		ET_WARNING("Expected an option ID but found: %s", arg.c_str());
		return;
	}
}

//----------------------------------
// CommandLineParser::ProcessOption
//
void CommandLineParser::ProcessOption(Option& option, std::string const& arg)
{
	TypeInfo const& ti = TypeInfoRegistry::Instance().GetTypeInfo(option.m_Type);

	switch (ti.m_Kind)
	{
	case TypeInfo::E_Kind::Arithmetic:
		switch (ti.m_Id.Get()) // switching through type ID is less safe but should allow for better optimizations
		{
		case "bool"_hash:
			if ((arg == "true") || (arg == "yes") || (arg == "t") || (arg == "y") || (arg == "1"))
			{
				*(reinterpret_cast<bool*>(option.m_Data)) = true;
				option.m_WasSet = true;
			}
			else if ((arg == "false") || (arg == "no") || (arg == "f") || (arg == "n") || (arg == "0"))
			{
				*(reinterpret_cast<bool*>(option.m_Data)) = false;
				option.m_WasSet = false;
			}

			break;

		case "char"_hash:
			if (arg.size() == 1u)
			{
				*(reinterpret_cast<char*>(option.m_Data)) = arg[0];
				option.m_WasSet = true;
			}

			break;

		case "char32_t"_hash:
			if (arg.size() == 1u)
			{
				*(reinterpret_cast<char32*>(option.m_Data)) = arg[0];
				option.m_WasSet = true;
			}

			break;

		case "signedchar"_hash:
			if (sscanf(arg.c_str(), "%hhi", reinterpret_cast<int8*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		case "short"_hash:
			if (sscanf(arg.c_str(), "%hi", reinterpret_cast<int16*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		case "int"_hash:
			if (sscanf(arg.c_str(), "%i", reinterpret_cast<int32*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		case "__int64"_hash:
			if (sscanf(arg.c_str(), "%li", reinterpret_cast<int32*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		case "unsignedchar"_hash:
			if (sscanf(arg.c_str(), "%hhi", reinterpret_cast<uint8*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		case "unsignedshort"_hash:
			if (sscanf(arg.c_str(), "%hi", reinterpret_cast<uint16*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		case "unsignedint"_hash:
			if (sscanf(arg.c_str(), "%i", reinterpret_cast<uint32*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		case "unsigned__int64"_hash:
			if (sscanf(arg.c_str(), "%li", reinterpret_cast<uint32*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		case "float"_hash:
			if (sscanf(arg.c_str(), "%f", reinterpret_cast<float*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		case "double"_hash:
			if (sscanf(arg.c_str(), "%lf", reinterpret_cast<double*>(option.m_Data)) == 1)
			{
				option.m_WasSet = true;
			}

			break;

		default:
			ET_ERROR("unhandled arithmetic type - id '%s'", ti.m_Id.ToStringDbg());
			break;
		}

		break;

	case TypeInfo::E_Kind::Enumeration:
		ET_WARNING("Enumerations aren't supported yet");
		break;

	case TypeInfo::E_Kind::Vector:
		ET_WARNING("Vectors aren't supported yet");
		break;

	case TypeInfo::E_Kind::String:
		*(reinterpret_cast<std::string*>(option.m_Data)) = arg;
		option.m_WasSet = true;
		break;

	case TypeInfo::E_Kind::Hash:
		*(reinterpret_cast<HashString*>(option.m_Data)) = HashString(arg.c_str());
		option.m_WasSet = true;
		break;

	default:
		ET_WARNING("Option %s has type %s which can't be processed", option.m_Id.ToStringDbg(), ti.m_Id.ToStringDbg());
		break;
	}

	if (!option.m_WasSet)
	{
		ET_WARNING("Option %s of type %s can't be set with '%s'", option.m_Id.ToStringDbg(), ti.m_Id.ToStringDbg(), arg.c_str());
	}
}


} // namespace core
} // namespace et
