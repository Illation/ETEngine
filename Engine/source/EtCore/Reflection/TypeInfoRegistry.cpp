#include "stdafx.h"
#include "TypeInfoRegistry.h"


namespace et {
namespace core {


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

		ET_TRACE_I(ET_CTX_CORE, "type: %s", ti.m_Id.ToStringDbg());
		for (TypeInfo::PropertyInfo const& myProp : ti.m_Properties)
		{
			ET_TRACE_I(ET_CTX_CORE, "\tproperty [%s]: %s", myProp.m_Id.ToStringDbg(), myProp.m_IsSerializable ? "RW" : "R_");
		}
	}
}


} // namespace core
} // namespace et
