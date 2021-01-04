#include "stdafx.h"
#include "BinarySerializer.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/IO/BinaryWriter.h>
#include <EtCore/Content/AssetPointer.h>

#include "TypeInfoRegistry.h"
#include "BinaryFormat.h"


namespace et {
namespace core {


//===================
// Binary Serializer
//===================


//-------------------------
// BinarySerializer::c-tor
//
// validate verbosity
//
BinarySerializer::BinarySerializer(bool const isVerbose)
#if ET_HASH_STRING_ENABLED
	: m_IsVerbose(isVerbose)
{}
#else
	: m_IsVerbose(false)
{
	ET_ASSERT(!isVerbose, "Hash to string conversion not supported, disabling verbose serialization");
}
#endif

//---------------------------------
// BinarySerializer::SerializeRoot
//
// Write the file header and recursively start writing
//
bool BinarySerializer::SerializeRoot(rttr::variant const& var, std::vector<uint8>& outData)
{
	m_Writer = new BinaryWriter(outData);

	// File Header
	m_Writer->IncreaseBufferSize(EtBin::s_Header.size() + build::Version::s_Name.size() + 1u + sizeof(uint8));

	m_Writer->WriteString(EtBin::s_Header);
	m_Writer->WriteNullString(build::Version::s_Name);
	m_Writer->Write<uint8>(m_IsVerbose ? 1u : 0u);

	// Write the object
	bool success = WriteVariant(var);

	// cleanup
	delete m_Writer;
	return success;
}

//--------------------------------
// BinarySerializer::WriteVariant
//
bool BinarySerializer::WriteVariant(rttr::variant const& var)
{
	rttr::type const valueType = var.get_type();
	if (valueType.is_wrapper())
	{
		return WriteVariant(var.extract_wrapped_value());
	}

	rttr::instance const inst(var);
	TypeInfo const& ti = TypeInfoRegistry::Instance().GetTypeInfo(inst.get_derived_type());

	// for pointers, write either null or the derived type ID
	if (valueType.is_pointer())
	{
		m_Writer->IncreaseBufferSize(sizeof(T_Hash));

		void* val = var.get_value<void*>();
		if (val == nullptr)
		{
			m_Writer->Write<T_Hash>(0u);
			return true;
		}

		m_Writer->Write(ti.m_Id.Get());
	}

	// handle different variant kinds
	switch (ti.m_Kind)
	{
	case TypeInfo::E_Kind::Arithmetic:
		return WriteArithmeticType(ti.m_Type, var);

	case TypeInfo::E_Kind::Enumeration:
		return WriteArithmeticType(ti.m_Type.get_enumeration().get_underlying_type(), var);

	case TypeInfo::E_Kind::Vector:
		return WriteVectorType(ti.m_Type, var);

	case TypeInfo::E_Kind::String:
		WriteString(var.to_string());
		return true;

	case TypeInfo::E_Kind::Hash:
		WriteHash(var.get_value<HashString>());
		return true;

	case TypeInfo::E_Kind::AssetPointer:
		WriteHash(var.get_value<I_AssetPtr>().GetId());
		return true;

	case TypeInfo::E_Kind::ContainerSequential:
		return WriteSequentialContainer(var.create_sequential_view(), true);

	case TypeInfo::E_Kind::ContainerAssociative:
		return WriteAssociativeContainer(var.create_associative_view());

	case TypeInfo::E_Kind::Class:
		return WriteObject(inst, ti);
	}

	ET_ASSERT(false, "unhandled variant type '%s'", ti.m_Id.ToStringDbg());
	return false;
}

//-------------------------------------
// BinarySerializer::WriteBasicVariant
//
// For non polymorphic or wrapped types where the TI is already known. 
//
bool BinarySerializer::WriteBasicVariant(rttr::variant const& var, TypeInfo const& ti)
{
	// handle different variant kinds
	switch (ti.m_Kind)
	{
	case TypeInfo::E_Kind::Arithmetic:
		return WriteArithmeticType(ti.m_Type, var);

	case TypeInfo::E_Kind::Enumeration:
		return WriteArithmeticType(ti.m_Type.get_enumeration().get_underlying_type(), var);

	case TypeInfo::E_Kind::Vector:
		return WriteVectorType(ti.m_Type, var);

	case TypeInfo::E_Kind::String:
		WriteString(var.to_string());
		return true;

	case TypeInfo::E_Kind::Hash:
		WriteHash(var.get_value<HashString>());
		return true;

	case TypeInfo::E_Kind::AssetPointer:
		WriteHash(var.get_value<I_AssetPtr>().GetId());
		return true;

	case TypeInfo::E_Kind::ContainerSequential:
		return WriteSequentialContainer(var.create_sequential_view(), true);

	case TypeInfo::E_Kind::ContainerAssociative:
		return WriteAssociativeContainer(var.create_associative_view());

	case TypeInfo::E_Kind::Class:
		return WriteObject(rttr::instance(var), ti);
	}

	ET_ASSERT(false, "unhandled variant type '%s'", ti.m_Id.ToStringDbg());
	return false;
}

//---------------------------------------
// BinarySerializer::WriteArithmeticType
//
bool BinarySerializer::WriteArithmeticType(rttr::type const type, rttr::variant const& var)
{
	if (type == rttr::type::get<bool>())
	{
		m_Writer->IncreaseBufferSize(sizeof(uint8));
		m_Writer->Write<uint8>(var.to_bool() ? 1u : 0u);
	}
	else if (type == rttr::type::get<char>())
	{
		m_Writer->IncreaseBufferSize(sizeof(uint8));
		m_Writer->Write(var.to_uint8());
	}
	else if (type == rttr::type::get<int8>())
	{
		m_Writer->IncreaseBufferSize(sizeof(int8));
		m_Writer->Write(var.to_int8());
	}
	else if (type == rttr::type::get<int16>())
	{
		m_Writer->IncreaseBufferSize(sizeof(int16));
		m_Writer->Write(var.to_int16());
	}
	else if (type == rttr::type::get<int32>())
	{
		m_Writer->IncreaseBufferSize(sizeof(int32));
		m_Writer->Write(var.to_int32());
	}
	else if (type == rttr::type::get<int64>())
	{
		m_Writer->IncreaseBufferSize(sizeof(int64));
		m_Writer->Write(var.to_int64());
	}
	else if (type == rttr::type::get<uint8>())
	{
		m_Writer->IncreaseBufferSize(sizeof(uint8));
		m_Writer->Write(var.to_uint8());
	}
	else if (type == rttr::type::get<uint16>())
	{
		m_Writer->IncreaseBufferSize(sizeof(uint16));
		m_Writer->Write(var.to_uint16());
	}
	else if (type == rttr::type::get<uint32>())
	{
		m_Writer->IncreaseBufferSize(sizeof(uint32));
		m_Writer->Write(var.to_uint32());
	}
	else if (type == rttr::type::get<uint64>())
	{
		m_Writer->IncreaseBufferSize(sizeof(uint64));
		m_Writer->Write(var.to_uint64());
	}
	else if (type == rttr::type::get<float>())
	{
		m_Writer->IncreaseBufferSize(sizeof(float));
		m_Writer->Write(var.to_float());
	}
	else if (type == rttr::type::get<double>())
	{
		m_Writer->IncreaseBufferSize(sizeof(double));
		m_Writer->Write(var.to_double());
	}
	else
	{
		ET_ASSERT(false, "unhandled arithmetic type '%s'", type.get_name().data());
		return false;
	}

	return true;
}

//-----------------------------------
// BinarySerializer::WriteVectorType
//
bool BinarySerializer::WriteVectorType(rttr::type const type, rttr::variant const& var)
{
	rttr::array_range<rttr::property> const prop_list = type.get_properties();
	ET_ASSERT(prop_list.size() == 1);

	rttr::property const prop = *prop_list.begin();
	ET_ASSERT(prop.get_name().to_string() == "data");

	rttr::instance const inst(var);
	rttr::type const propValType = prop.get_type();
	rttr::variant const propVar = prop.get_value(inst);

	ET_ASSERT(propValType.is_sequential_container());
	return WriteSequentialContainer(propVar.create_sequential_view(), false);
}

//-------------------------------
// BinarySerializer::WriteString
//
void BinarySerializer::WriteString(std::string const& str)
{
	m_Writer->IncreaseBufferSize(str.size() + 1u);
	m_Writer->WriteNullString(str);
}

//-----------------------------
// BinarySerializer::WriteHash
//
void BinarySerializer::WriteHash(HashString const hash)
{
	if (m_IsVerbose) 
	{
		m_Writer->IncreaseBufferSize(sizeof(uint8)); // we store a bool determining if an int or a string should be read

		char const* const str = hash.GetStoredString();
		if (str != nullptr)
		{
			ET_ASSERT(hash.Get() == GetHash(str));

			m_Writer->Write<uint8>(1u);

			WriteString(str);
			return;
		}
		else
		{
			m_Writer->Write<uint8>(0u);
		}
	}

	m_Writer->IncreaseBufferSize(sizeof(T_Hash));
	m_Writer->Write(hash.Get());
}

//--------------------------------------------
// BinarySerializer::WriteSequentialContainer
//
bool BinarySerializer::WriteSequentialContainer(rttr::variant_sequential_view const& view, bool const writeCount)
{
	uint64 const itemCount = static_cast<uint64>(view.get_size());
	if (writeCount)
	{
		m_Writer->IncreaseBufferSize(sizeof(itemCount));
		m_Writer->Write(itemCount);
	}

	rttr::type const itemType = view.get_value_type();
	if (IsBasic(itemType))
	{
		TypeInfo const& ti = TypeInfoRegistry::Instance().GetTypeInfo(itemType);

		for (size_t idx = 0u; idx < static_cast<size_t>(itemCount); ++idx)
		{
			if (!WriteBasicVariant(view.get_value(idx), ti))
			{
				ET_ASSERT(false,
					"Failed to write basic element of sequential container type '%s' at index " ET_FMT_SIZET,
					view.get_type().get_name().data(),
					idx);
				return false;
			}
		}
	}
	else
	{
		for (size_t idx = 0u; idx < static_cast<size_t>(itemCount); ++idx)
		{
			if (!WriteVariant(view.get_value(idx)))
			{
				ET_ASSERT(false, 
					"Failed to write element of sequential container type '%s' at index " ET_FMT_SIZET, 
					view.get_type().get_name().data(), 
					idx);
				return false;
			}
		}
	}

	return true;
}

//---------------------------------------------
// BinarySerializer::WriteAssociativeContainer
//
bool BinarySerializer::WriteAssociativeContainer(rttr::variant_associative_view const& view)
{
	uint64 const itemCount = static_cast<uint64>(view.get_size());
	m_Writer->IncreaseBufferSize(sizeof(itemCount));
	m_Writer->Write(itemCount);

	rttr::type const keyType = view.get_key_type();
	if (IsBasic(keyType))
	{
		TypeInfo const& keyTi = TypeInfoRegistry::Instance().GetTypeInfo(keyType);

		if (view.is_key_only_type())
		{
			for (auto& item : view)
			{
				if (!WriteBasicVariant(item.first, keyTi))
				{
					ET_ASSERT(false, "Failed to write element of key only associative container type '%s'", view.get_type().get_name().data());
					return false;
				}
			}
		}
		else
		{
			rttr::type const valueType = view.get_value_type();
			if (IsBasic(valueType))
			{
				TypeInfo const& valueTi = TypeInfoRegistry::Instance().GetTypeInfo(valueType);

				for (auto& item : view)
				{
					if (!WriteBasicVariant(item.first, keyTi))
					{
						ET_ASSERT(false, "Failed to write key of associative container type '%s'", view.get_type().get_name().data());
						return false;
					}

					if (!WriteBasicVariant(item.second, valueTi))
					{
						ET_ASSERT(false, "Failed to write basic value of associative container type '%s'", view.get_type().get_name().data());
						return false;
					}
				}
			}
			else
			{
				for (auto& item : view)
				{
					if (!WriteBasicVariant(item.first, keyTi))
					{
						ET_ASSERT(false, "Failed to write key of associative container type '%s'", view.get_type().get_name().data());
						return false;
					}

					if (!WriteVariant(item.second))
					{
						ET_ASSERT(false, "Failed to write value of associative container type '%s'", view.get_type().get_name().data());
						return false;
					}
				}
			}
		}
	}
	else
	{
		if (view.is_key_only_type())
		{
			for (auto& item : view)
			{
				if (!WriteVariant(item.first))
				{
					ET_ASSERT(false, "Failed to write element of key only associative container type '%s'", view.get_type().get_name().data());
					return false;
				}
			}
		}
		else
		{
			rttr::type const valueType = view.get_value_type();
			if (IsBasic(valueType))
			{
				TypeInfo const& valueTi = TypeInfoRegistry::Instance().GetTypeInfo(valueType);

				for (auto& item : view)
				{
					if (!WriteVariant(item.first))
					{
						ET_ASSERT(false, "Failed to write key of associative container type '%s'", view.get_type().get_name().data());
						return false;
					}

					if (!WriteBasicVariant(item.second, valueTi))
					{
						ET_ASSERT(false, "Failed to write basic value of associative container type '%s'", view.get_type().get_name().data());
						return false;
					}
				}
			}
			else
			{
				for (auto& item : view)
				{
					if (!WriteVariant(item.first))
					{
						ET_ASSERT(false, "Failed to write key of associative container type '%s'", view.get_type().get_name().data());
						return false;
					}

					if (!WriteVariant(item.second))
					{
						ET_ASSERT(false, "Failed to write value of associative container type '%s'", view.get_type().get_name().data());
						return false;
					}
				}
			}
		}
	}

	return true;
}

//-------------------------------
// BinarySerializer::WriteObject
//
bool BinarySerializer::WriteObject(rttr::instance const& inst, TypeInfo const& ti)
{
	// write property count
	uint16 propCount = 0u;
	for (TypeInfo::PropertyInfo const& prop : ti.m_Properties)
	{
		if (prop.m_IsSerializable)
		{
			propCount++;
		}
	}

	// we can preemptively allocate size for property IDs
	m_Writer->IncreaseBufferSize(sizeof(propCount) + (static_cast<size_t>(propCount) * sizeof(T_Hash)));
	m_Writer->Write(propCount);

	// write properties
	for (TypeInfo::PropertyInfo const& prop : ti.m_Properties)
	{
		if (!prop.m_IsSerializable)
		{
			continue;
		}

		rttr::variant const propVal = prop.m_Property.get_value(inst);
		if (!propVal)
		{
			// handle nullptr here?
			ET_ASSERT(false, "failed to get value for property '%s' in type '%s'", prop.m_Id.ToStringDbg(), ti.m_Id.ToStringDbg());
			return false;
		}

		m_Writer->Write(prop.m_Id.Get());

		if (!WriteVariant(propVal))
		{
			ET_ASSERT(false, "failed to write property '%s' in type '%s'", prop.m_Id.ToStringDbg(), ti.m_Id.ToStringDbg());
			return false;
		}
	}

	return true;
}

//---------------------------
// BinarySerializer::IsBasic
//
bool BinarySerializer::IsBasic(rttr::type const type) const
{
	return (!type.is_pointer() && !type.is_wrapper());
}


} // namespace core
} // namespace et
