#include "stdafx.h"
#include "CommandLineParser.h"

#include <EtCore/Reflection/TypeInfoRegistry.h>
#include <EtCore/Util/DebugCommandController.h>


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
			if (ProcessOption(*currentOption, arg))
			{
				IdentifyOption(arg, currentOption);
			}
			else
			{
				currentOption = nullptr;
			}
		}
	}

	if (currentOption != nullptr)
	{
		if (currentOption->m_Type == rttr::type::get<bool>())
		{
			*(reinterpret_cast<bool*>(currentOption->m_Data)) = true;
			currentOption->m_WasSet = true;
		}
		else
		{
			ET_LOG_W(ET_CTX_CORE, "Last option didn't have an argument but wasn't of type bool, type: %s", currentOption->m_Type.get_name().data());
		}
	}

	m_HasProcessed = true;

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	dbg::CommandController::Instance().AddCommand(dbg::Command("core_print_command_line", "Print state and description of command line options"),
		dbg::T_CommandFn([this](dbg::Command const& command, std::string const& parameters)
			{
				ET_UNUSED(command);
				ET_UNUSED(parameters);
				PrintAll(true);
				return dbg::E_CommandRes::Success;
			}));
#endif
}

//-----------------------------
// CommandLineParser::PrintAll
//
void CommandLineParser::PrintAll(bool const showState) const
{
	ET_LOG_I(ET_CTX_CORE, "");
	ET_LOG_I(ET_CTX_CORE, "Command line options:");

	for (Option const& option : m_Options)
	{
		ET_LOG_I(ET_CTX_CORE, "\t%s--%s <%s>\t\t\t\t%s",
			(option.m_ShortHand == 0) ? "" : FS("-%c, ", option.m_ShortHand).c_str(),
			option.m_Id.ToStringDbg(),
			option.m_Type.get_name().data(),
			option.m_Description.c_str());

		if (showState && option.m_WasSet)
		{
			if (option.m_Type == rttr::type::get<bool>())
			{
				ET_LOG_I(ET_CTX_CORE, "\t\tset: %s", *(reinterpret_cast<bool*>(option.m_Data)) ? "true" : "false");
			}
			else
			{
				ET_LOG_I(ET_CTX_CORE, "\t\tset: %s", option.m_ProcessedArg.c_str());
			}
		}
	}
}

//-------------------------------------------
// CommandLineParser::RegisterOptionInternal
//
void CommandLineParser::RegisterOptionInternal(uintptr_t const data, 
	rttr::type const type,
	T_VariantAssignFn const assignFn,
	std::string const& name, 
	char const shorthand,
	std::string const& desc)
{
	HashString const optionId(name.c_str());

	auto foundIt = std::find_if(m_Options.cbegin(), m_Options.cend(), [optionId](Option const& option)
		{
			return (option.m_Id == optionId);
		});

	if (foundIt != m_Options.cend())
	{
		ET_WARNING("Option '%s' already set", name.c_str());
		return;
	}

	if (shorthand != 0)
	{
		auto foundIt = std::find_if(m_Options.cbegin(), m_Options.cend(), [shorthand](Option const& option)
			{
				return (option.m_ShortHand == shorthand);
			});

		if (foundIt != m_Options.cend())
		{
			ET_WARNING("Shorthand '%c' already used", shorthand);
			return;
		}
	}

	m_Options.emplace_back(type);
	Option& option = m_Options.back();

	option.m_Id = optionId;
	option.m_ShortHand = shorthand;
	option.m_AssignFn = assignFn;

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
// return true to skip
//
bool CommandLineParser::ProcessOption(Option& option, std::string const& arg)
{
	TypeInfo const& ti = TypeInfoRegistry::Instance().GetTypeInfo(option.m_Type);

	switch (ti.m_Kind)
	{
	case TypeInfo::E_Kind::Arithmetic:
		if (ProcessArithmeticOption(option, arg, ti.m_Id))
		{
			return true;
		}

		break;

	case TypeInfo::E_Kind::Enumeration:
	{
		rttr::variant const var = option.m_Type.get_enumeration().name_to_value(arg.c_str());
		if (option.m_AssignFn(option.m_Data, var))
		{
			option.m_WasSet = true;
		}

		break;
	}

	case TypeInfo::E_Kind::Vector:
		ProcessVectorOption(option, arg, ti.m_Id);
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

	if (option.m_WasSet)
	{
		option.m_ProcessedArg = arg;
	}
	else
	{
		ET_WARNING("Option %s of type %s can't be set with '%s'", option.m_Id.ToStringDbg(), ti.m_Id.ToStringDbg(), arg.c_str());
	}

	return false;
}

//--------------------------------------------
// CommandLineParser::ProcessArithmeticOption
//
bool CommandLineParser::ProcessArithmeticOption(Option& option, std::string const& arg, HashString const typeId)
{
	switch (typeId.Get()) // switching through type ID is less safe but should allow for better optimizations
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
			option.m_WasSet = true;
		}
		else
		{
			*(reinterpret_cast<bool*>(option.m_Data)) = true;
			option.m_WasSet = true;
			return true;
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
		ET_ERROR("unhandled arithmetic type - id '%s'", typeId.ToStringDbg());
		break;
	}

	return false;
}

//--------------------------------------------
// CommandLineParser::ProcessArithmeticOption
//
void CommandLineParser::ProcessVectorOption(Option &option, std::string const &arg, HashString const typeId)
{
	std::vector<std::string> const separated = SeparateListArg(arg);

	switch (typeId.Get()) // switching through type ID is less safe but should allow for better optimizations
	{
	case "ivec2"_hash:
		if (ProcessVectorInternal(separated, "%i", *reinterpret_cast<ivec2*>(option.m_Data)))
		{
			option.m_WasSet = true;
		}

		break;

	case "vec2"_hash:
		if (ProcessVectorInternal(separated, "%f", *reinterpret_cast<vec2*>(option.m_Data)))
		{
			option.m_WasSet = true;
		}

		break;

	case "vec3"_hash:
		if (ProcessVectorInternal(separated, "%f", *reinterpret_cast<vec3*>(option.m_Data)))
		{
			option.m_WasSet = true;
		}

		break;

	case "vec4"_hash:
		if (ProcessVectorInternal(separated, "%f", *reinterpret_cast<vec4*>(option.m_Data)))
		{
			option.m_WasSet = true;
		}

		break;

	default:
		ET_ERROR("unhandled vector type - id '%s'", typeId.ToStringDbg());
		break;
	}
}

//------------------------------------
// CommandLineParser::SeparateListArg
//
std::vector<std::string> CommandLineParser::SeparateListArg(std::string const &arg) const
{
	std::vector<std::string> ret;
	if (arg.empty())
	{
		return ret;
	}

	size_t sectionStart = 0u;
	size_t pos = 0u;
	for (; pos < arg.size(); ++pos)
	{
		if (arg[pos] == ',')
		{
			if (pos > sectionStart)
			{
				ret.push_back(arg.substr(sectionStart, pos - sectionStart));
			}

			sectionStart = pos + 1;
		}
	}

	if ((arg[pos - 1] != ',') && (pos > sectionStart))
	{
		ret.push_back(arg.substr(sectionStart));
	}

	return ret;
}


} // namespace core
} // namespace et
