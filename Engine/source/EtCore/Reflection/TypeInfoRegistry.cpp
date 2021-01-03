#include "stdafx.h"
#include "TypeInfoRegistry.h"


namespace et {
namespace core {


//===========
// Type Info
//===========


//-------------------------------
// TypeInfo::PropertyInfo::c-tor
//
TypeInfo::PropertyInfo::PropertyInfo(rttr::property const prop)
	: m_Id(prop.get_name().data())
	, m_Property(prop)
{ }

//-----------------
// TypeInfo::c-tor
//
TypeInfo::TypeInfo(rttr::type const type)
	: m_Id(type.get_name().data())
	, m_Type(type)
{
	rttr::array_range<rttr::property> const properties = m_Type.get_properties();
	for (rttr::property const prop : properties)
	{
		m_Properties.emplace_back(prop);
	}
}

//-------------------------
// TypeInfo::GetProperties
//
rttr::property const* TypeInfo::GetProperty(HashString const id) const
{
	auto const foundIt = std::find_if(m_Properties.cbegin(), m_Properties.cend(), [id](PropertyInfo const& info)
		{
			return (info.m_Id == id);
		});

	if (foundIt == m_Properties.cend())
	{
		return nullptr;
	}

	return &(foundIt->m_Property);
}


//====================
// Type Info Registry
//====================


//----------------------------
// TypeInfoRegistry::Instance
//
// Global access
//
TypeInfoRegistry& TypeInfoRegistry::Instance()
{
	static TypeInfoRegistry instance;
	return instance;
}

//------------------------------
// TypeInfoRegistry::Initialize
//
// Generate IDs for all registered types
//
void TypeInfoRegistry::Initialize()
{
	rttr::array_range<rttr::type> const allTypes = rttr::type::get_types();
	for (rttr::type const myType : allTypes)
	{
		TypeInfo const info(myType);
		std::pair<T_TypeInfoMap::iterator, bool> const result = m_Types.try_emplace(info.m_Id, info);
		if (result.second)
		{
			auto const result2 = m_TypeAccess.try_emplace(myType, result.first);
			ET_ASSERT(result2.second, "already had an access for type '%s'", myType.get_name().data());
		}
	}
}

//-------------------------------
// TypeInfoRegistry::GetTypeInfo
//
TypeInfo const* TypeInfoRegistry::GetTypeInfo(HashString const typeId) const
{
	auto const foundIt = m_Types.find(typeId);
	if (foundIt == m_Types.cend())
	{
		return nullptr;
	}

	return &(foundIt->second);
}

//-------------------------------
// TypeInfoRegistry::GetTypeInfo
//
TypeInfo const& TypeInfoRegistry::GetTypeInfo(rttr::type const type) const
{
	auto const foundIt = m_TypeAccess.find(type);
	ET_ASSERT(foundIt != m_TypeAccess.cend());

	return foundIt->second->second;
}

//-------------------------------
// TypeInfoRegistry::DbgPrintAll
//
void TypeInfoRegistry::DbgPrintAll() const
{
	for (auto const el : m_Types)
	{
		TypeInfo const& ti = el.second;

		LOG(FS("type: %s", ti.m_Id.ToStringDbg()));
		for (TypeInfo::PropertyInfo const& myProp : ti.m_Properties)
		{
			LOG(FS("\tproperty: %s", myProp.m_Id.ToStringDbg()));
		}
	}
}


} // namespace core
} // namespace et
