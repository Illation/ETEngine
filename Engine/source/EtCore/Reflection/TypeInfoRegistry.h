#pragma once


namespace et {
namespace core {


//------------------
// TypeInfo
//
// Includes rttr type info combined with IDs that are stable for serialization
//
struct TypeInfo final
{
	// definitions
	//-------------
	struct PropertyInfo
	{
		PropertyInfo(rttr::property const prop);

		HashString m_Id;
		rttr::property m_Property;
	};

	// functionality
	//---------------
	TypeInfo(rttr::type const type);

	rttr::property const* GetProperty(HashString const id) const;

	// Data
	///////

	HashString m_Id;

	rttr::type m_Type;
	std::vector<PropertyInfo> m_Properties;
};

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

