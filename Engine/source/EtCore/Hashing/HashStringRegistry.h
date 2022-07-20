#pragma once
#include "Hash.h"

#include <EtCore/Containers/linear_hash_map.h>
#include <EtCore/Util/DebugUtilFwd.h>


// enable to detect strings which result in the same hash as they are registered
#define ET_CT_DETECT_HASHSTRING_COLLISIONS ET_DISABLED
#define ET_CT_VERIFY_HASHSTRING_REGISTRATION ET_DISABLED | ET_CT_IS_ENABLED(ET_CT_DETECT_HASHSTRING_COLLISIONS)


namespace et {
namespace core {


//--------------------
// HashStringRegistry
//
// Database for hashing results (for debug purposes)
//
class HashStringRegistry final
{
	// static access
	//---------------
public:
	static HashStringRegistry& Instance();

	// construct destruct
	//--------------------
private:
	HashStringRegistry();
public:
	HashStringRegistry(HashStringRegistry const&) = delete;
	void operator=(HashStringRegistry const&) = delete;

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	void InitDebugCommands();
#endif

	// functionality
	//---------------
	void Register(T_Hash const hash, char const* const str);

	void DbgPrintAll() const;
	void DbgCheckCollisions() const;

	// accessors
	//-----------
	char const* GetString(T_Hash const hash) const;
	bool const HasHash(T_Hash const hash) const;

	// Data
	///////

private:
	core::lin_hash_map<T_Hash, std::string> m_RegisteredHashes;
};


} // namespace core
} // namespace et

