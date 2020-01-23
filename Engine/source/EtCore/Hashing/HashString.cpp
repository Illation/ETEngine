#include "stdafx.h"	
#include "HashString.h"

#include "HashStringRegistry.h"


namespace et {
namespace core {


//=============
// Hash String
//=============


//------------------------
// HashString::c-tor
//
// construct from a pre-hashed value
//
HashString::HashString(T_Hash const val)
	: m_Hash(val)
{ }

//------------------------
// HashString::c-tor
//
// construct from a string, hashing it, and - in debug - registering the string in the hash DB
//
HashString::HashString(char const* const source)
	: m_Hash(GetHash(source))
{
#if ET_HASH_STRING_ENABLED	
	HashStringRegistry::Instance().Register(m_Hash, source);
#endif
}

//----------------
// HashString:: = 
//
// assign from pre-hashed value
//
HashString& HashString::operator=(T_Hash const val)
{
	Set(val); 
	return *this;
}

//----------------
// HashString:: = 
//
// assign from string (and register the string)
//
HashString& HashString::operator=(char const* const source)
{
	Set(source); 
	return *this;
}

//-----------------
// HashString::Set
//
// hash the string and register it in the DB in non shipping builds
//
void HashString::Set(char const* const source)
{
	m_Hash = GetHash(source);

#if ET_HASH_STRING_ENABLED	
	HashStringRegistry::Instance().Register(m_Hash, source);
#endif
}


// debug
///////////

#if ET_HASH_STRING_ENABLED	

//--------------------------
// HashString::GetStringDbg
//
// in non shipping builds we can acquire a string from the hashed value
//  this should not be used for logic, only logging purposes and serializing / deserializing non shipping content
//
char const* HashString::GetStringDbg() const
{
	return HashStringRegistry::Instance().GetString(m_Hash);
}

#endif


} // namespace core
} // namespace et
