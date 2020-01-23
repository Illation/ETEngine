#pragma once
#include "Hash.h"


// enable to detect strings which result in the same hash as they are registered
#define ET_DETECT_HASHSTRING_COLLISIONS false
#define ET_VERIFY_HASHSTRING_REGISTRATION (false | ET_DETECT_HASHSTRING_COLLISIONS)


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
	HashStringRegistry() = default;
public:
	HashStringRegistry(HashStringRegistry const&) = delete;
	void operator=(HashStringRegistry const&) = delete;

	// functionality
	//---------------
	void Register(T_Hash const hash, char const* const str);

	// accessors
	//-----------
	char const* GetString(T_Hash const hash) const;

	// Data
	///////

private:
	std::unordered_map<T_Hash, std::string> m_RegisteredHashes;
};


} // namespace core
} // namespace et

