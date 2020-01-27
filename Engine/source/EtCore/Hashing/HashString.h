#pragma once
#include "Hash.h"

#include <EtCore/Util/LinkerUtils.h>


// definitions
//-------------

#ifdef ET_SHIPPING
	#define ET_HASH_STRING_ENABLED false
#else
	#define ET_HASH_STRING_ENABLED true
#endif


namespace et {
namespace core {


class HashStringRegistry;


//------------------------
// HashString
//
// Utility hash that registers its name in a database for debugging purposes and is serialized / deserialized as an atomic type
//
class HashString final
{
	// definitions
	//-------------
	DECLARE_FORCED_LINKING()

#if ET_HASH_STRING_ENABLED
	static HashStringRegistry* const s_GlobalHashStringRegistry;
#endif

	static std::string s_LastStringResult;

	// construct destruct
	//--------------------
public:
	HashString() = default;
	explicit HashString(T_Hash const val);
#if ET_HASH_STRING_ENABLED
	explicit HashString(char const* const source);
#else
	constexpr explicit HashString(char const* const source) : m_Hash(GetHash(source)) {}

	template <unsigned int TCount>
	constexpr explicit HashString(char const(&str)[TCount]) : m_Hash(detail::hash_gen(str, TCount)) {}
#endif

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
	char const* GetStoredString() const;
#endif
	char const* ToStringDbg() const;

	// Data
	///////

private:
	T_Hash m_Hash = 0u;
};


} // namespace core
} // namespace et


namespace std {

//------------------------
// std::hash<HashString>
//
// Allow using a hash string as a key for maps
//
template <>
struct hash<et::core::HashString>
{
	std::size_t operator()(et::core::HashString const& hs) const { return static_cast<std::size_t>(hs.Get()); }
};

} // namespace std
