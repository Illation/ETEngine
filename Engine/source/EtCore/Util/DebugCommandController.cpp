#include "stdafx.h"
#include "DebugCommandController.h"

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)

#include <cctype>
#include <locale>


namespace et {
namespace core {

namespace dbg {


//==========================
// Debug Command Controller
//==========================


// static
std::string const CommandController::s_HelpRequestParm("-h");

//-----------------------------
// CommandController::Instance
//
// Global singleton access
//
CommandController& CommandController::Instance()
{
	static CommandController instance;
	return instance;
}


//-------------------------------
// CommandController::AddCommand
//
void CommandController::AddCommand(Command const& cmd, T_CommandFn const& fn)
{
	core::HashString const id(cmd.m_Name.c_str());
	ET_ASSERT(GetCommandIt(id) == m_Commands.cend(), "Command '%s' already exists", cmd.m_Name.c_str());

	m_Commands.emplace(id, StoredCommand(cmd, fn));
	m_CommandInfoSet.insert(CommandIdInfo(cmd.m_Name, id));
}

//-----------------------------------
// CommandController::ExecuteCommand
//
E_CommandRes CommandController::ExecuteCommand(std::string const& commandString, core::HashString& commandId) const
{
	// parse command ID
	commandId.Reset();
	size_t const separator = commandString.find(' ');
	bool hasParameters = false;
	if (separator == std::string::npos)
	{
		commandId = core::HashString(commandString.c_str());
	}
	else
	{
		commandId = core::HashString(commandString.substr(0, separator).c_str());
		if (separator < (commandString.size() - 1))
		{
			hasParameters = true;
		}
	}

	// find the relevant command
	T_StoredCommands::const_iterator const cmdIt = GetCommandIt(commandId);
	if (cmdIt == m_Commands.cend())
	{
		return E_CommandRes::NotFound;
	}

	// get the parameters
	std::string const parameters(hasParameters ? commandString.substr(separator + 1u) : "");

	// check if we need to print the help string
	if (hasParameters)
	{
		bool matches = true;
		size_t i = 0u;
		for (; i < s_HelpRequestParm.size(); ++i)
		{
			if (i >= parameters.size() || (parameters[i] != s_HelpRequestParm[i]))
			{
				matches = false;
				break;
			}
		}

		if (matches && ((i >= parameters.size()) || (std::isspace(parameters[i]))))
		{
			return E_CommandRes::PrintHelp;
		}
	}

	// exectue the command
	StoredCommand const& cmd = cmdIt->second;
	return cmd.m_CommandFn(cmd.m_Command, parameters);
}

//-------------------------------
// CommandController::GetCommand
//
Command const* CommandController::GetCommand(core::HashString const id) const
{
	T_StoredCommands::const_iterator const cmdIt = GetCommandIt(id);
	if (cmdIt != m_Commands.cend())
	{
		return &(cmdIt->second.m_Command);
	}

	return nullptr;
}

//---------------------------------
// CommandController::GetCommandIt
//
CommandController::T_StoredCommands::const_iterator CommandController::GetCommandIt(core::HashString const id) const
{
	return m_Commands.find(id);
}


} // namespace dbg

} // namespace core
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
