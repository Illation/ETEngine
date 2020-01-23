#pragma once
#include "Hash.h"

// definitions
//-------------

#ifdef ET_SHIPPING
	#define ET_HASH_STRING_ENABLED true
#else
	#define ET_HASH_STRING_ENABLED false
#endif


namespace et {
namespace core {


//------------------------
// HashString
//
// Utility hash that registers its name in a database for debugging purposes and is serialized / deserialized as an atomic type
//
class HashString final
{
	// construct destruct
	//--------------------
public:
	HashString() = default;
	explicit HashString(T_Hash const val);
	explicit HashString(char const* const source);

	HashString& operator=(T_Hash const val);
	HashString& operator=(char const* const source);

	// functionality
	//----------------
	void Reset() { m_Hash = 0u; }
	void Set(T_Hash const val) { m_Hash = val; }
	void Set(char const* const source);

	// operators
	//-----------
	bool operator== (T_Hash const other) const { return (m_Hash == other); }
	bool operator== (HashString const other) const { return (m_Hash == other.Get()); }

	bool operator<= (T_Hash const other) const { return (m_Hash <= other); }
	bool operator<= (HashString const other) const { return (m_Hash <= other.Get()); }

	bool operator>= (T_Hash const other) const { return (m_Hash >= other); }
	bool operator>= (HashString const other) const { return (m_Hash >= other.Get()); }

	bool operator!= (T_Hash const other) const { return (m_Hash != other); }
	bool operator!= (HashString const other) const { return (m_Hash != other.Get()); }

	bool operator< (T_Hash const other) const { return (m_Hash < other); }
	bool operator< (HashString const other) const { return (m_Hash < other.Get()); }

	bool operator> (T_Hash const other) const { return (m_Hash > other); }
	bool operator> (HashString const other) const { return (m_Hash > other.Get()); }

	// accessors
	//-----------
	T_Hash Get() const { return m_Hash; }
	bool IsEmpty() const { return m_Hash == 0u; }

	// debug
	//-------
#if ET_HASH_STRING_ENABLED	
	char const* GetStringDbg() const;
#endif

	// Data
	///////

private:
	T_Hash m_Hash = 0u;
};


} // namespace core
} // namespace et
