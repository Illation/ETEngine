#pragma once
#include "DebugUtilFwd.h"

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)


namespace et {
namespace core {

namespace dbg {


//--------------------
// E_CommandRes
//
// The result of executing a debug command
//
enum class E_CommandRes: uint8
{
	Success,
	Error,
	IncorrecParameters,

	NotFound, // returned by controller not user
	PrintHelp
};

//--------------------
// Command
//
// Command data
//
struct Command final
{
	Command(std::string const& name, std::string const& usage) : m_Name(name), m_Usage(usage) {}

	std::string const m_Name;
	std::string const m_Usage;
};

//--------------
// T_CommandFn
//
typedef std::function<E_CommandRes(Command const& command, std::string const& parameters)> T_CommandFn;


//--------------------
// CommandIdInfo
//
// Utility info for autocompleting commands
//
struct CommandIdInfo
{
	CommandIdInfo(std::string const& name, core::HashString const id) : m_Name(name), m_Id(id) {}

	bool operator==(CommandIdInfo const& rhs) const { return m_Id == rhs.m_Id; }
	bool operator<(CommandIdInfo const& rhs) const { return m_Name < rhs.m_Name; }

	std::string m_Name;
	core::HashString m_Id;
};


} // namespace dbg

} // namespace core
} // namespace et



namespace std {

//------------------------
// std::hash<et::core::dbg::CommandController::CommandIdInfo>
//
template <>
struct hash<et::core::dbg::CommandIdInfo>
{
	std::size_t operator()(et::core::dbg::CommandIdInfo const& lhs) const { return static_cast<std::size_t>(lhs.m_Id.Get()); }
};

} // namespace std


namespace et {
namespace core {

namespace dbg {

typedef std::set<CommandIdInfo> T_CommandInfoSet;


//--------------------
// CommandController
//
// Collects and has functionality for executing debug commands
//
class CommandController final
{
	// definitions
	//-------------
	struct StoredCommand
	{
		StoredCommand(Command const& cmd, T_CommandFn const& fn) : m_Command(cmd), m_CommandFn(fn) {}

		Command m_Command;
		T_CommandFn m_CommandFn;
	};

	typedef std::unordered_map<core::HashString, StoredCommand> T_StoredCommands;

public:

	static std::string const s_HelpRequestParm;

	// static
	//--------
	static CommandController& Instance();

	// construct destruct
	//--------------------
private:
	CommandController() = default;

public:
	CommandController(CommandController const&) = delete;
	void operator=(CommandController const&) = delete;

	// functionality
	//---------------
	void AddCommand(Command const& cmd, T_CommandFn const& fn);

	E_CommandRes ExecuteCommand(std::string const& commandString, core::HashString& commandId) const;

	// accessors
	//-----------
	Command const* GetCommand(core::HashString const id) const;
	T_CommandInfoSet const& GetCommandInfoSet() const { return m_CommandInfoSet; }

	// utility
	//---------
private:
	T_StoredCommands::const_iterator GetCommandIt(core::HashString const id) const;


	// Data
	///////

	T_StoredCommands m_Commands;
	T_CommandInfoSet m_CommandInfoSet;
};


} // namespace dbg

} // namespace core
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
