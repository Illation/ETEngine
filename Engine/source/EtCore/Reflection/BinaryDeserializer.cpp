#include "stdafx.h"
#include "BinaryDeserializer.h"

#include <EtBuild/EngineVersion.h>

#include "TypeInfoRegistry.h"
#include "BinaryFormat.h"


namespace et {
namespace core {


//=====================
// Binary Deserializer
//=====================


//-------------------------------------
// BinaryDeserializer::DeserializeRoot
//
bool BinaryDeserializer::DeserializeRoot(rttr::variant& var, rttr::type const callingType, std::vector<uint8> const& data)
{
	// setup
	m_Reader.Open(data);
	ET_ASSERT(m_Reader.Exists());

	// read header
	if (m_Reader.ReadString(EtBin::s_Header.size()) != EtBin::s_Header)
	{
		ET_ASSERT(false, "Incorrect header for ETBIN file");
		m_Reader.Close();
		return false;
	}

	std::string const writerVersion = m_Reader.ReadNullString();
	if (writerVersion != build::Version::s_Name)
	{
		LOG(FS("etbin was written by a different engine version: %s - calling type '%s'", writerVersion.c_str(), callingType.get_name().data()));
	}

	m_IsVerbose = (m_Reader.Read<uint8>() != 0u);

	// main deserialization
	bool const success = ReadVariant(var, callingType);

	// cleanup
	m_Reader.Close();
	return success;
}

//---------------------------------
// BinaryDeserializer::ReadVariant
//
bool BinaryDeserializer::ReadVariant(rttr::variant& var, rttr::type const callingType)
{
	if (callingType.is_wrapper())
	{
		return ReadVariant(var, callingType.get_wrapped_type());
	}

	if (callingType.is_pointer()) // we need to allocate pointer types on the heap
	{
		HashString const typeId(m_Reader.Read<T_Hash>());
		if (typeId.IsEmpty()) // nullptr
		{
			var = nullptr;
			return true;
		}

		TypeInfo const* const ti = TypeInfoRegistry::Instance().GetTypeInfo(typeId);
		if (ti == nullptr)
		{
			ET_ASSERT(false, "Couldn't get type info from ID '%s'", typeId.ToStringDbg());
			return false;
		}

		if (!(ti->m_Type.is_derived_from(callingType)))
		{
			ET_ASSERT(false, 
				"Serialized type '%s' doesn't derive from calling type '%s'", 
				ti->m_Type.get_name().data(), 
				callingType.get_name().data());
			return false;
		}

		rttr::constructor copyCtor = ti->m_Type.get_constructor({ti->m_Type});
		if (!copyCtor.is_valid())
		{
			ET_ASSERT(false, "no valid default constructor found for serialized type '%s'", ti->m_Type.get_name().data());
			return false;
		}

		rttr::variant innerVar;
		if (!ReadBasicVariant(innerVar, *ti))
		{
			ET_ASSERT(false, "failed to read inner type '%s' of pointer type '%s'", ti->m_Type.get_name().data(), callingType.get_name().data());
			return false;
		}

		// call copy constructor for our type -> it will be constructed on the heap
		var = copyCtor.invoke(innerVar);

		if (!var.convert(callingType))
		{
			ET_ASSERT(false, "failed to convert inner type '%s' to pointer type '%s'", ti->m_Type.get_name().data(), callingType.get_name().data());
			return false;
		}

		return true;
	}

	return ReadBasicVariant(var, TypeInfoRegistry::Instance().GetTypeInfo(callingType));
}

//--------------------------------------
// BinaryDeserializer::ReadBasicVariant
//
bool BinaryDeserializer::ReadBasicVariant(rttr::variant& var, TypeInfo const& ti)
{
	// handle different variant kinds
	switch (ti.m_Kind)
	{
	case TypeInfo::E_Kind::Arithmetic:
		return false;

	case TypeInfo::E_Kind::Enumeration:
		return false;

	case TypeInfo::E_Kind::Vector:
		return false;

	case TypeInfo::E_Kind::String:
		return false;

	case TypeInfo::E_Kind::Hash:
		return false;

	case TypeInfo::E_Kind::AssetPointer:
		return false;

	case TypeInfo::E_Kind::ContainerSequential:
		return false;

	case TypeInfo::E_Kind::ContainerAssociative:
		return false;

	case TypeInfo::E_Kind::Class:
		return false;
	}

	ET_ASSERT(false, "unhandled variant type '%s'", ti.m_Id.ToStringDbg());
	return false;
}


} // namespace core
} // namespace et
