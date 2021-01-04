#pragma once
#include "TypeInfo.h"


namespace et {
namespace core {


//------------------
// TypeInfoRegistry
//
// Globally accessible container for type info
//
class TypeInfoRegistry final
{
	// definitions
	//------------
	typedef std::unordered_map<HashString, TypeInfo> T_TypeInfoMap;

	// static access
	//---------------
public:
	static TypeInfoRegistry& Instance();

	// construct destruct
	//--------------------
	TypeInfoRegistry() = default;

	void Initialize();

	// accessors
	//-----------
	TypeInfo const* GetTypeInfo(HashString const typeId) const;
	TypeInfo const& GetTypeInfo(rttr::type const type) const;

	// utility
	//---------
	void DbgPrintAll() const;

	// Data
	///////

private:
	T_TypeInfoMap m_Types;
	std::unordered_map<rttr::type, T_TypeInfoMap::const_iterator> m_TypeAccess;
};


} // namespace core
} // namespace et

