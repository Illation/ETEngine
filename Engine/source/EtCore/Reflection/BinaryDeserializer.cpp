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

		rttr::variant innerVar = var;
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
		return ReadArithmeticType(var, ti.m_Type);

	case TypeInfo::E_Kind::Enumeration:
		if (ReadArithmeticType(var, ti.m_Type.get_enumeration().get_underlying_type()))
		{
			if (var.convert(ti.m_Type))
			{
				return true;
			}
		}

		return false;

	case TypeInfo::E_Kind::Vector:
		return ReadVectorType(var, ti.m_Id);

	case TypeInfo::E_Kind::String:
		var = m_Reader.ReadNullString();
		return true;

	case TypeInfo::E_Kind::Hash:
		ReadHash(var);
		return true;

	case TypeInfo::E_Kind::AssetPointer:
		ReadHash(var);
		if (var.convert(ti.m_Type))
		{
			return true;
		}

		return false;

	case TypeInfo::E_Kind::ContainerSequential:
		return ReadSequentialContainer(var);

	case TypeInfo::E_Kind::ContainerAssociative:
		return ReadAssociativeContainer(var);

	case TypeInfo::E_Kind::Class:
		return ReadObject(var, ti);
	}

	ET_ASSERT(false, "unhandled variant type '%s'", ti.m_Id.ToStringDbg());
	return false;
}

//----------------------------------------
// BinaryDeserializer::ReadArithmeticType
//
bool BinaryDeserializer::ReadArithmeticType(rttr::variant& var, rttr::type const type)
{
	if (type == rttr::type::get<bool>())
	{
		var = (m_Reader.Read<uint8>() != 0u);
	}
	else if (type == rttr::type::get<char>())
	{
		var = static_cast<char>(m_Reader.Read<uint8>());
	}
	else if (type == rttr::type::get<int8>())
	{
		var = m_Reader.Read<int8>();
	}
	else if (type == rttr::type::get<int16>())
	{
		var = m_Reader.Read<int16>();
	}
	else if (type == rttr::type::get<int32>())
	{
		var = m_Reader.Read<int32>();
	}
	else if (type == rttr::type::get<int64>())
	{
		var = m_Reader.Read<int64>();
	}
	else if (type == rttr::type::get<uint8>())
	{
		var = m_Reader.Read<uint8>();
	}
	else if (type == rttr::type::get<uint16>())
	{
		var = m_Reader.Read<uint16>();
	}
	else if (type == rttr::type::get<uint32>())
	{
		var = m_Reader.Read<uint32>();
	}
	else if (type == rttr::type::get<uint64>())
	{
		var = m_Reader.Read<uint64>();
	}
	else if (type == rttr::type::get<float>())
	{
		var = m_Reader.Read<float>();
	}
	else if (type == rttr::type::get<double>())
	{
		var = m_Reader.Read<double>();
	}
	else
	{
		ET_ASSERT(false, "unhandled arithmetic type '%s'", type.get_name().data());
		return false;
	}

	return true;
}

//------------------------------------
// BinaryDeserializer::ReadVectorType
//
bool BinaryDeserializer::ReadVectorType(rttr::variant& var, HashString const typeId)
{
	switch (typeId.Get()) // switching through type ID is less safe but should allow for better optimizations
	{
	case "vec2"_hash:
		var = ReadVector<2, float>();
		break;
	case "vec3"_hash:
		var = ReadVector<3, float>();
		break;
	case "vec4"_hash:
		var = ReadVector<4, float>();
		break;
	case "ivec2"_hash:
		var = ReadVector<2, int32>();
		break;
	case "ivec3"_hash:
		var = ReadVector<3, int32>();
		break;
	case "ivec4"_hash:
		var = ReadVector<4, int32>();
		break;
	case "dvec2"_hash:
		var = ReadVector<2, double>();
		break;
	case "dvec3"_hash:
		var = ReadVector<3, double>();
		break;
	case "dvec4"_hash:
		var = ReadVector<4, double>();
		break;

	case "mat2"_hash:
		var = ReadMatrix<2, 2, float>();
		break;
	case "mat3"_hash:
		var = ReadMatrix<3, 3, float>();
		break;
	case "mat4"_hash:
		var = ReadMatrix<4, 4, float>();
		break;
	case "dmat2"_hash:
		var = ReadMatrix<2, 2, double>();
		break;
	case "dmat3"_hash:
		var = ReadMatrix<3, 3, double>();
		break;
	case "dmat4"_hash:
		var = ReadMatrix<4, 4, double>();
		break;

	case "quat"_hash:
	{
		quat q;
		m_Reader.ReadData(reinterpret_cast<uint8*>(q.data.data()), sizeof(float) * 4u);
		var = q;
		break;
	}
	case "quatd"_hash:
	{
		quatd qd;
		m_Reader.ReadData(reinterpret_cast<uint8*>(qd.data.data()), sizeof(double) * 4u);
		var = qd;
		break;
	}

	default:
		ET_ASSERT(false, "unhandled vector type - id '%s'", typeId.ToStringDbg());
		return false;
	}

	return true;
}

//------------------------------
// BinaryDeserializer::ReadHash
//
void BinaryDeserializer::ReadHash(rttr::variant& var)
{
	if (m_IsVerbose)
	{
		if (m_Reader.Read<uint8>() != 0)
		{
			var = HashString(m_Reader.ReadNullString().c_str());
		}
	}

	var = HashString(m_Reader.Read<T_Hash>());
}

//---------------------------------------------
// BinaryDeserializer::ReadSequentialContainer
//
bool BinaryDeserializer::ReadSequentialContainer(rttr::variant& var)
{
	rttr::variant_sequential_view view = var.create_sequential_view();
	
	// ensure the container has the correct size
	size_t const count = static_cast<size_t>(m_Reader.Read<uint64>());
	if (view.get_size() != count)
	{
		view.set_size(count);
	}

	rttr::type const valueType = view.get_value_type();

	for (size_t idx = 0u; idx < count; ++idx)
	{
		rttr::variant elementVar = view.get_value(idx);
		if (!ReadVariant(elementVar, valueType))
		{
			ET_ASSERT(false,
				"Failed to read element from sequential container (type: %s) at index [" ET_FMT_SIZET "]",
				view.get_type().get_name().data(),
				idx);
			return false;
		}

		if (!view.set_value(idx, elementVar))
		{
			return false;
		}
	}

	return true;
}

//----------------------------------------------
// BinaryDeserializer::ReadAssociativeContainer
//
bool BinaryDeserializer::ReadAssociativeContainer(rttr::variant& var)
{
	rttr::variant_associative_view view = var.create_associative_view();

	size_t const count = static_cast<size_t>(m_Reader.Read<uint64>());

	rttr::type const keyType = view.get_key_type();
	if (view.is_key_only_type())
	{
		for (size_t idx = 0u; idx < count; ++idx)
		{
			rttr::variant key;
			if (!ReadVariant(key, keyType))
			{
				ET_ASSERT(false,
					"Failed to read element from key only associative container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}

			if (!key.convert(keyType))
			{
				ET_ASSERT(false,
					"Failed to convert element from key only associative container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}

			if (!view.insert(key).second)
			{
				ET_ASSERT(false,
					"Failed to insert element from key only associative container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}
		}
	}
	else
	{
		rttr::type const valueType = view.get_value_type();

		for (size_t idx = 0u; idx < count; ++idx)
		{
			rttr::variant key; 
			if (!ReadVariant(key, keyType))
			{
				ET_ASSERT(false,
					"Failed to read key from associative container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}

			if (!key.convert(keyType))
			{
				ET_ASSERT(false,
					"Failed to convert key from associative container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}

			rttr::variant value; // not setting this from existing data might cause a problem in the future if there are nested containers
			if (!ReadVariant(value, valueType))
			{
				ET_ASSERT(false,
					"Failed to read value from associative container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}

			if (!value.convert(valueType))
			{
				ET_ASSERT(false,
					"Failed to convert value from associative container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}

			if (!view.insert(key, value).second)
			{
				ET_ASSERT(false,
					"Failed to insert keyval pair from associative container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}
		}
	}

	return true;
}

//--------------------------------
// BinaryDeserializer::ReadObject
//
bool BinaryDeserializer::ReadObject(rttr::variant& var, TypeInfo const& ti)
{
	uint16 const propCount = m_Reader.Read<uint16>();

	if (propCount > 0u)
	{
		rttr::instance inst(var);

		for (uint16 propIdx = 0u; propIdx < propCount; ++propIdx)
		{
			// get property from ID
			HashString const propId(m_Reader.Read<T_Hash>());
			rttr::property const* const propPtr = ti.GetProperty(propId);
			if (propPtr == nullptr)
			{
				ET_ASSERT(false, "Couldn't get property with ID '%s' from type '%s'", propId.ToStringDbg(), ti.m_Type.get_name().data());
				return false;
			}

			rttr::property const prop = *propPtr;
			rttr::type const propType = prop.get_type();

			// read current value so the property variant has type info on it
			rttr::variant propVar = prop.get_value(inst);
			if (!propVar)
			{
				ET_ASSERT(false, "Couldn't get property value '%s' from instance of type '%s'", propId.ToStringDbg(), ti.m_Type.get_name().data());
				return false;
			}

			// read serialized value
			if (!ReadVariant(propVar, propType) || !propVar.is_valid())
			{
				ET_ASSERT(false, "Couldn't read property value '%s' - type '%s' - deserializing type '%s'", 
					propId.ToStringDbg(), 
					propType.get_name().data(),
					ti.m_Type.get_name().data());
				return false;
			}

			// apply to instance
			if (propVar.get_type() == rttr::type::get<std::nullptr_t>())
			{
				prop.set_value(inst, nullptr);
			}
			else
			{
				prop.set_value(inst, propVar);
			}
		}
	}

	return true;
}


} // namespace core
} // namespace et
