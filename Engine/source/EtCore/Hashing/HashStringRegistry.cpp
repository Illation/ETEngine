#include "stdafx.h"	
#include "HashStringRegistry.h"


namespace et {
namespace core {


//======================
// Hash String Registry
//======================


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

//------------------------------
// HashStringRegistry::Register
//
// Register a hash and it's string value
//
void HashStringRegistry::Register(T_Hash const hash, char const* const str)
{
	// ensure this is a valid hash
#if ET_VERIFY_HASHSTRING_REGISTRATION	
	ET_ASSERT(hash == GetHash(str));
#endif

	// check that any existing value has the same hash
#if ET_DETECT_HASHSTRING_COLLISIONS

	auto const foundIt = m_RegisteredHashes.find(hash);
	if (foundIt != m_RegisteredHashes.cend())
	{
		ET_ASSERT(foundIt->second == str, "Hash collision detected, '%s', '%s'", foundIt->second.c_str(), str);
	}

#endif

	// add the hash pair - emplace doesn't do anything if it was already added previously
	m_RegisteredHashes.emplace(hash, str);
}

//-------------------------------
// HashStringRegistry::GetString
//
// Access the string value associated with a hash
//
char const* HashStringRegistry::GetString(T_Hash const hash) const
{
	auto const foundIt = m_RegisteredHashes.find(hash);

	if (foundIt != m_RegisteredHashes.cend())
	{
		return foundIt->second.c_str();
	}

	return nullptr;
}


} // namespace core
} // namespace et
