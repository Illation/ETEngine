#include "stdafx.h"	
#include "HashStringRegistry.h"

#include <EtCore/Util/DebugCommandController.h>


namespace et {
namespace core {


//======================
// Hash String Registry
//======================


//------------------------------
// HashStringRegistry::c-tor
//
// Init hash map - #todo: 16 buckets for now but we should probably find a better bucket count
//
HashStringRegistry::HashStringRegistry() 
	: m_RegisteredHashes(16u, 0u)
{ }

//------------------------------
// HashStringRegistry::Instance
//
// Global singleton access
//
HashStringRegistry& HashStringRegistry::Instance()
{
	static HashStringRegistry instance;
	return instance;
}

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
//---------------------------------------
// HashStringRegistry::InitDebugCommands
//
void HashStringRegistry::InitDebugCommands()
{
	dbg::CommandController& cmdController = dbg::CommandController::Instance();

	cmdController.AddCommand(dbg::Command("core_print_all_hashstrings", "Print all known hash strings"),
		dbg::T_CommandFn([this](dbg::Command const& command, std::string const& parameters)
			{
				ET_UNUSED(command);
				ET_UNUSED(parameters);
				DbgPrintAll();
				return dbg::E_CommandRes::Success;
			}));

	cmdController.AddCommand(dbg::Command("core_check_hashstring_collisions", "Print any colliding hash strings"),
		dbg::T_CommandFn([this](dbg::Command const& command, std::string const& parameters)
			{
				ET_UNUSED(command);
				ET_UNUSED(parameters);
				DbgCheckCollisions();
				return dbg::E_CommandRes::Success;
			}));
}
#endif

//------------------------------
// HashStringRegistry::Register
//
// Register a hash and it's string value
//
void HashStringRegistry::Register(T_Hash const hash, char const* const str)
{
	if (hash == 0u)
	{
		return;
	}

	// ensure this is a valid hash
#if ET_CT_IS_ENABLED(ET_CT_VERIFY_HASHSTRING_REGISTRATION)
	ET_ASSERT(hash == GetHash(str));
#endif

	// check that any existing value has the same hash
#if ET_CT_IS_ENABLED(ET_CT_DETECT_HASHSTRING_COLLISIONS)

	auto const foundIt = m_RegisteredHashes.find(hash);
	if (foundIt != m_RegisteredHashes.cend())
	{
		ET_ASSERT(foundIt->second == str, "Hash collision detected, '%s', '%s'", foundIt->second.c_str(), str);
	}

#endif

	// add the hash pair - emplace doesn't do anything if it was already added previously
	m_RegisteredHashes.emplace(hash, str);
}

//---------------------------------
// HashStringRegistry::DbgPrintAll
//
void HashStringRegistry::DbgPrintAll() const
{
	ET_TRACE_I(ET_CTX_CORE, "");
	ET_TRACE_I(ET_CTX_CORE, "Cached hashes:");

	for (std::pair<T_Hash const, std::string> const& pair : m_RegisteredHashes)
	{
		ET_TRACE_I(ET_CTX_CORE, "\t[%u] - '%s'", pair.first, pair.second.c_str());
	}
}

//----------------------------------------
// HashStringRegistry::DbgCheckCollisions
//
void HashStringRegistry::DbgCheckCollisions() const
{
	ET_TRACE_I(ET_CTX_CORE, "Checking for hash string collisions ...");

	bool hasCollisions = false;
	for (std::pair<T_Hash const, std::string> const& pair : m_RegisteredHashes)
	{
		for (std::pair<T_Hash const, std::string> const& innerPair : m_RegisteredHashes)
		{
			if ((pair.first == innerPair.first) && (pair.second != innerPair.second))
			{
				ET_TRACE_W(ET_CTX_CORE, "\t[%u] - '%s', '%s'", pair.first, pair.second.c_str(), innerPair.second.c_str());
				hasCollisions = true;
			}
		}
	}

	if (hasCollisions)
	{
		ET_TRACE_E(ET_CTX_CORE, "Hash collisions detected!");
	}
	else
	{
		ET_TRACE_I(ET_CTX_CORE, "No hash collisions detected");
	}
}

//-------------------------------
// HashStringRegistry::GetString
//
// Access the string value associated with a hash
//
char const* HashStringRegistry::GetString(T_Hash const hash) const
{
	if (hash == 0u)
	{
		return nullptr;
	}

	auto const foundIt = m_RegisteredHashes.find(hash);

	if (foundIt != m_RegisteredHashes.cend())
	{
		return foundIt->second.c_str();
	}

	return nullptr;
}

//-------------------------------
// HashStringRegistry::GetString
//
bool const HashStringRegistry::HasHash(T_Hash const hash) const
{
	return (m_RegisteredHashes.find(hash) != m_RegisteredHashes.cend());
}


} // namespace core
} // namespace et
