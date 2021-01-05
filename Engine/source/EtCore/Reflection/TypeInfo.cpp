#include "stdafx.h"
#include "TypeInfo.h"

#include <EtCore/Content/AssetRegistration.h>

#include "registerMath.h"


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
	, m_IsSerializable(!prop.get_metadata("NO_SERIALIZE"))
{ }

//-------------------
// TypeInfo::IsBasic
//
bool TypeInfo::IsBasic(rttr::type const type)
{
	return (!type.is_pointer() && !type.is_wrapper());
}

//-----------------
// TypeInfo::c-tor
//
TypeInfo::TypeInfo(rttr::type const type)
	: m_Id(type.get_name().data())
	, m_Type(type)
{
	if (type.is_arithmetic())
	{
		m_Kind = E_Kind::Arithmetic;
	}
	else if (type.is_enumeration())
	{
		m_Kind = E_Kind::Enumeration;
	}
	else if (IsVectorType(type))
	{
		m_Kind = E_Kind::Vector;
	}
	else if (type == rttr::type::get<std::string>())
	{
		m_Kind = E_Kind::String;
	}
	else if (type == rttr::type::get<HashString>())
	{
		m_Kind = E_Kind::Hash;
	}
	else if (IsSerializableAssetPointerType(type))
	{
		m_Kind = E_Kind::AssetPointer;
	}
	else if (type.is_sequential_container())
	{
		m_Kind = E_Kind::ContainerSequential;
	}
	else if (type.is_associative_container())
	{
		m_Kind = E_Kind::ContainerAssociative;
	}
	else if (type.is_class())
	{
		m_Kind = E_Kind::Class;
	}

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


} // namespace core
} // namespace et
