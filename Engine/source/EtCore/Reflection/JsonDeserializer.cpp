#include "stdafx.h"
#include "JsonDeserializer.h"

#include <EtCore/IO/JsonDom.h>
#include <EtCore/IO/JsonWriter.h>
#include <EtCore/Hashing/HashString.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/AssetRegistration.h>

#include "registerMath.h"
#include "TypeInfoRegistry.h"


namespace et {
namespace core {


//===================
// JSON Deserializer
//===================


//-----------------------------------
// JsonDeserializer::DeserializeRoot
//
// For any type
//
bool JsonDeserializer::DeserializeRoot(rttr::variant& var, rttr::type const callingType, JSON::Object const* const parentObj)
{
	return ReadVariant(var, callingType, parentObj);
}

//-----------------------------------
// JsonDeserializer::DeserializeRoot
//
// For deserializing a non pointer object
//
bool JsonDeserializer::DeserializeRoot(rttr::instance& inst, TypeInfo const& ti, JSON::Object const* const parentObj)
{
	ET_ASSERT(parentObj->value.size() == 1u);
	ET_ASSERT(parentObj->value[0].second->GetType() == JSON::JSON_Object);

	rttr::type instType = inst.get_type().get_raw_type().is_wrapper() ? inst.get_wrapped_instance().get_derived_type() : inst.get_derived_type();
	if (parentObj->value[0].first != instType.get_name().to_string())
	{
		ET_ASSERT(false, "Expected object instance type to match the serialized value type");
		return false;
	}

	return ReadObjectProperties(inst, ti, parentObj->value[0].second->obj());
}

//-------------------------------
// JsonDeserializer::ReadVariant
//
bool JsonDeserializer::ReadVariant(rttr::variant& var, rttr::type const callingType, JSON::Value const* const jVal)
{
	if (callingType.is_wrapper())
	{
		if (!ReadVariant(var, callingType.get_wrapped_type(), jVal))
		{
			ET_ASSERT(false, "Failed to read wrapped variant content");
			return false;
		}

		// convert from inner type to pointer type
		if (!var.convert(callingType))
		{
			ET_ASSERT(false, 
				"failed to convert inner type '%s' to wrapper type '%s'", 
				callingType.get_wrapped_type().get_name().data(), 
				callingType.get_name().data());
			return false;
		}

		return true;
	}

	if (callingType.is_pointer())
	{
		if (jVal->GetType() == JSON::JSON_Null)
		{
			var = nullptr;
			return true;
		}
		else if (jVal->GetType() != JSON::JSON_Object)
		{
			ET_ASSERT(false, "Expected JSON::Value representing a pointer to be of type Object (with a single internal value denoting the type");
			return false;
		}

		// value is stored in a nested object member with the member name denoting the type ID and the member type containing the object data
		JSON::Object const* const jObj = jVal->obj();
		ET_ASSERT(jObj->value.size() == 1u, "Expected pointer JSON object to have exactly one internal value!");

		HashString const typeId(jObj->value[0].first.c_str());

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

		// call default constructor for our type -> it will be constructed on the heap
		rttr::constructor defCtor = ti->m_Type.get_constructor();
		if (!defCtor.is_valid())
		{
			ET_ASSERT(false, "no valid default constructor found for serialized type '%s'", ti->m_Type.get_name().data());
			return false;
		}

		var = defCtor.invoke();

		// deserialize the object content into newly allocated memory
		if (!ReadBasicVariant(var, *ti, jObj->value[0].second))
		{
			ET_ASSERT(false, "failed to read inner type '%s' of pointer type '%s'", ti->m_Type.get_name().data(), callingType.get_name().data());
			return false;
		}

		// convert from inner type to pointer type
		if (!var.convert(callingType))
		{
			ET_ASSERT(false, "failed to convert inner type '%s' to pointer type '%s'", ti->m_Type.get_name().data(), callingType.get_name().data());
			return false;
		}

		return true;
	}

	return ReadBasicVariant(var, TypeInfoRegistry::Instance().GetTypeInfo(callingType), jVal);
}

//------------------------------------
// JsonDeserializer::ReadBasicVariant
//
bool JsonDeserializer::ReadBasicVariant(rttr::variant& var, TypeInfo const& ti, JSON::Value const* const jVal)
{	// handle different variant kinds
	switch (ti.m_Kind)
	{
	case TypeInfo::E_Kind::Arithmetic:
		return ReadArithmeticType(var, ti.m_Id, jVal);

	case TypeInfo::E_Kind::Enumeration:
		return ReadEnum(var, ti.m_Type, jVal);

	case TypeInfo::E_Kind::Vector:
		return ReadVectorType(var, ti, jVal);

	case TypeInfo::E_Kind::String:
		ET_ASSERT(jVal->GetType() == JSON::JSON_String);
		var = jVal->str()->value;
		return true;

	case TypeInfo::E_Kind::Hash:
		return ReadHash(var, jVal);

	case TypeInfo::E_Kind::AssetPointer:
		if (ReadHash(var, jVal))
		{
			if (var.convert(ti.m_Type))
			{
				return true;
			}
		}

		return false;

	case TypeInfo::E_Kind::ContainerSequential:
		return ReadSequentialContainer(var, jVal);

	case TypeInfo::E_Kind::ContainerAssociative:
		return ReadAssociativeContainer(var, jVal);

	case TypeInfo::E_Kind::Class:
		return ReadObject(var, ti, jVal);
	}

	ET_ASSERT(false, "unhandled variant type '%s'", ti.m_Id.ToStringDbg());
	return false;
}

//--------------------------------------
// JsonDeserializer::ReadArithmeticType
//
bool JsonDeserializer::ReadArithmeticType(rttr::variant& var, HashString const typeId, JSON::Value const* const jVal)
{
	if (typeId == "bool"_hash)
	{
		ET_ASSERT(jVal->GetType() == JSON::JSON_Bool);
		var = jVal->b()->value;
		return true;
	}

	ET_ASSERT(jVal->GetType() == JSON::JSON_Number);

	switch (typeId.Get()) // switching through type ID is less safe but should allow for better optimizations
	{
	case "char"_hash:
		var = static_cast<char>(jVal->num()->valueInt);
		break;

	case "signedchar"_hash:
		var = static_cast<int8>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;
	case "short"_hash:
		var = static_cast<int16>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;
	case "int"_hash:
		var = static_cast<int32>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;
	case "__int64"_hash:
		var = static_cast<int64>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;

	case "unsignedchar"_hash:
		var = static_cast<uint8>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;
	case "unsignedshort"_hash:
		var = static_cast<uint16>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;
	case "unsignedint"_hash:
		var = static_cast<uint32>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;
	case "unsigned__int64"_hash:
		var = static_cast<uint64>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;

	case "float"_hash:
		var = static_cast<float>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;
	case "double"_hash:
		var = static_cast<double>(jVal->num()->isInt ? jVal->num()->valueInt : jVal->num()->value);
		break;

	default:
		ET_ASSERT(false, "unhandled arithmetic type - id '%s'", typeId.ToStringDbg());
		return false;
	}

	return true;
}

//----------------------------
// JsonDeserializer::ReadEnum
//
bool JsonDeserializer::ReadEnum(rttr::variant& var, rttr::type const enumType, JSON::Value const* const jVal)
{
	if (jVal->GetType() == JSON::JSON_String)
	{
		var = enumType.get_enumeration().name_to_value(jVal->str()->value.c_str());
		return true;
	}
	else
	{
		if (ReadArithmeticType(var, TypeInfoRegistry::Instance().GetTypeInfo(enumType.get_enumeration().get_underlying_type()).m_Id, jVal))
		{
			if (var.convert(enumType))
			{
				return true;
			}
		}
	}

	return false;
}

//----------------------------------
// JsonDeserializer::ReadVectorType
//
bool JsonDeserializer::ReadVectorType(rttr::variant& var, TypeInfo const& ti, JSON::Value const* const jVal)
{
	rttr::property const* const prop = ti.GetProperty(HashString("data"_hash));
	ET_ASSERT(prop != nullptr);

	rttr::type propValType = prop->get_type();
	ET_ASSERT(propValType.is_sequential_container());

	rttr::instance inst(var); // creating var from existing instance helps deduce the type even without deserializing
	rttr::variant propVar = prop->get_value(inst);
	if (!ReadSequentialContainer(propVar, jVal) || !propVar.is_valid())
	{
		ET_ASSERT(false, "There was an issue deserializing the vectors / matrices sequential view, typeName: '%s'!", ti.m_Id.ToStringDbg());
		return false;
	}

	prop->set_value(inst, propVar);
	return true;
}

//----------------------------
// JsonDeserializer::ReadHash
//
bool JsonDeserializer::ReadHash(rttr::variant& var, JSON::Value const* const jVal)
{
	if (jVal->GetType() == JSON::JSON_String)
	{
		var = HashString(jVal->str()->value.c_str());
		return true;
	}
	else if (jVal->GetType() == JSON::JSON_Number)
	{
		JSON::Number const* const jNum = jVal->num();
		if (jNum->isInt)
		{
			var = HashString(static_cast<T_Hash>(jNum->valueInt));
		}
		else
		{
			var = HashString(static_cast<T_Hash>(jNum->value));
		}

		return true;
	}

	return false;
}

//-------------------------------------------
// JsonDeserializer::ReadSequentialContainer
//
bool JsonDeserializer::ReadSequentialContainer(rttr::variant& var, JSON::Value const* const jVal)
{
	ET_ASSERT(jVal->GetType() == JSON::JSON_Array);
	JSON::Array const* const jArr = jVal->arr();

	rttr::variant_sequential_view view = var.create_sequential_view();

	// ensure the container has the correct size
	if (view.get_size() != jArr->value.size())
	{
		view.set_size(jArr->value.size());
	}

	if (jArr->value.empty())
	{
		return true;
	}

	rttr::type const valueType = view.get_value_type();
	if (TypeInfo::IsBasic(valueType))
	{
		// for basic types we can fetch type info once instead of for every array element
		TypeInfo const& valueTi = TypeInfoRegistry::Instance().GetTypeInfo(valueType);

		for (size_t idx = 0u; idx < jArr->value.size(); ++idx)
		{
			rttr::variant elementVar = view.get_value(idx).extract_wrapped_value();
			if (!ReadBasicVariant(elementVar, valueTi, jArr->value[idx]) || !elementVar.is_valid())
			{
				ET_ASSERT(false,
					"Failed to read element from sequential container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}

			ET_ASSERT(elementVar.get_type() == valueType);

			if (!view.set_value(idx, elementVar))
			{
				ET_ASSERT(false,
					"Failed to set value in sequential container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}
		}
	}
	else
	{
		for (size_t idx = 0u; idx < jArr->value.size(); ++idx)
		{
			rttr::variant elementVar = view.get_value(idx).extract_wrapped_value();
			if (!ReadVariant(elementVar, valueType, jArr->value[idx]) || !elementVar.is_valid())
			{
				ET_ASSERT(false,
					"Failed to read element from sequential container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}

			rttr::type const varType = elementVar.get_type();
			ET_ASSERT(varType == valueType);

			if (!view.set_value(idx, elementVar))
			{
				ET_ASSERT(false,
					"Failed to set value in sequential container (type: %s) at index [" ET_FMT_SIZET "]",
					view.get_type().get_name().data(),
					idx);
				return false;
			}
		}
	}

	return true;
}

//--------------------------------------------
// JsonDeserializer::ReadAssociativeContainer
//
bool JsonDeserializer::ReadAssociativeContainer(rttr::variant& var, JSON::Value const* const jVal)
{
	ET_ASSERT(jVal->GetType() == JSON::JSON_Array);
	JSON::Array const* const jArr = jVal->arr();

	rttr::variant_associative_view view = var.create_associative_view();
	view.clear();

	if (jArr->value.empty())
	{
		return true;
	}

	auto getKeyValFn = [jArr](size_t const idx, JSON::Value const*& key, JSON::Value const*& value) -> bool
		{
			JSON::Value const* const jVal = jArr->value[idx];
			if (jVal->GetType() != JSON::JSON_Object)
			{
				ET_ASSERT(false, "Expected associative container elements to be JSON objects");
				return false;
			}

			JSON::Object const* const jObj = jVal->obj();

			// extract the key
			auto jKeyIt = std::find_if(jObj->value.begin(), jObj->value.end(), [](JSON::Pair const& el)
				{
					return el.first == std::string("key");
				});

			if (jKeyIt == jObj->value.cend())
			{
				ET_ASSERT(false, "Failed to find key in sequential container at index " ET_FMT_SIZET, idx);
				return false;
			}

			key = jKeyIt->second;

			// extract the value
			auto jValIt = std::find_if(jObj->value.begin(), jObj->value.end(), [](JSON::Pair const& el)
				{
					return el.first == std::string("value");
				});

			if (jValIt == jObj->value.cend())
			{
				ET_ASSERT(false, "Failed to find value in sequential container at index " ET_FMT_SIZET, idx);
				return false;
			}

			value = jValIt->second;

			return true;
		};

	rttr::type const keyType = view.get_key_type();
	if (TypeInfo::IsBasic(keyType))
	{
		TypeInfo const& keyTi = TypeInfoRegistry::Instance().GetTypeInfo(keyType);

		if (view.is_key_only_type())
		{
			for (size_t idx = 0u; idx < jArr->value.size(); ++idx)
			{
				rttr::variant key;
				if (!ReadBasicVariant(key, keyTi, jArr->value[idx]))
				{
					ET_ASSERT(false,
						"Failed to read basic element from key only associative container (type: %s) at index [" ET_FMT_SIZET "]",
						view.get_type().get_name().data(),
						idx);
					return false;
				}

				ET_ASSERT(key.get_type() == keyType);

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
			if (TypeInfo::IsBasic(valueType))
			{
				TypeInfo const& valueTi = TypeInfoRegistry::Instance().GetTypeInfo(valueType);

				for (size_t idx = 0u; idx < jArr->value.size(); ++idx)
				{
					JSON::Value const* jKeyEl;
					JSON::Value const* jValEl;
					if (!getKeyValFn(idx, jKeyEl, jValEl))
					{
						return false; // lambda will already have asserted why
					}

					rttr::variant key;
					if (!ReadBasicVariant(key, keyTi, jKeyEl))
					{
						ET_ASSERT(false,
							"Failed to read basic key from associative container (type: %s) at index [" ET_FMT_SIZET "]",
							view.get_type().get_name().data(),
							idx);
						return false;
					}

					ET_ASSERT(key.get_type() == keyType);

					rttr::variant value; // not setting this from existing data might cause a problem in the future if there are nested containers
					if (!ReadBasicVariant(value, valueTi, jValEl))
					{
						ET_ASSERT(false,
							"Failed to read basic value from associative container (type: %s) at index [" ET_FMT_SIZET "]",
							view.get_type().get_name().data(),
							idx);
						return false;
					}

					ET_ASSERT(value.get_type() == valueType);

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
			else
			{
				for (size_t idx = 0u; idx < jArr->value.size(); ++idx)
				{
					JSON::Value const* jKeyEl;
					JSON::Value const* jValEl;
					if (!getKeyValFn(idx, jKeyEl, jValEl))
					{
						return false; // lambda will already have asserted why
					}

					rttr::variant key;
					if (!ReadBasicVariant(key, keyTi, jKeyEl))
					{
						ET_ASSERT(false,
							"Failed to read basic key from associative container (type: %s) at index [" ET_FMT_SIZET "]",
							view.get_type().get_name().data(),
							idx);
						return false;
					}

					ET_ASSERT(key.get_type() == keyType);

					rttr::variant value; // not setting this from existing data might cause a problem in the future if there are nested containers
					if (!ReadVariant(value, valueType, jValEl))
					{
						ET_ASSERT(false,
							"Failed to read value from associative container (type: %s) at index [" ET_FMT_SIZET "]",
							view.get_type().get_name().data(),
							idx);
						return false;
					}

					ET_ASSERT(value.get_type() == valueType);

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
		}
	}
	else
	{
		if (view.is_key_only_type())
		{
			for (size_t idx = 0u; idx < jArr->value.size(); ++idx)
			{
				rttr::variant key;
				if (!ReadVariant(key, keyType, jArr->value[idx]))
				{
					ET_ASSERT(false,
						"Failed to read element from key only associative container (type: %s) at index [" ET_FMT_SIZET "]",
						view.get_type().get_name().data(),
						idx);
					return false;
				}

				ET_ASSERT(key.get_type() == keyType);

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
			if (TypeInfo::IsBasic(valueType))
			{
				TypeInfo const& valueTi = TypeInfoRegistry::Instance().GetTypeInfo(valueType);

				for (size_t idx = 0u; idx < jArr->value.size(); ++idx)
				{
					JSON::Value const* jKeyEl;
					JSON::Value const* jValEl;
					if (!getKeyValFn(idx, jKeyEl, jValEl))
					{
						return false; // lambda will already have asserted why
					}

					rttr::variant key;
					if (!ReadVariant(key, keyType, jKeyEl))
					{
						ET_ASSERT(false,
							"Failed to read key from associative container (type: %s) at index [" ET_FMT_SIZET "]",
							view.get_type().get_name().data(),
							idx);
						return false;
					}

					ET_ASSERT(key.get_type() == keyType);

					rttr::variant value; // not setting this from existing data might cause a problem in the future if there are nested containers
					if (!ReadBasicVariant(value, valueTi, jValEl))
					{
						ET_ASSERT(false,
							"Failed to read basic value from associative container (type: %s) at index [" ET_FMT_SIZET "]",
							view.get_type().get_name().data(),
							idx);
						return false;
					}

					ET_ASSERT(value.get_type() == valueType);

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
			else
			{
				for (size_t idx = 0u; idx < jArr->value.size(); ++idx)
				{
					JSON::Value const* jKeyEl;
					JSON::Value const* jValEl;
					if (!getKeyValFn(idx, jKeyEl, jValEl))
					{
						return false; // lambda will already have asserted why
					}

					rttr::variant key;
					if (!ReadVariant(key, keyType, jKeyEl))
					{
						ET_ASSERT(false,
							"Failed to read key from associative container (type: %s) at index [" ET_FMT_SIZET "]",
							view.get_type().get_name().data(),
							idx);
						return false;
					}

					ET_ASSERT(key.get_type() == keyType);

					rttr::variant value; // not setting this from existing data might cause a problem in the future if there are nested containers
					if (!ReadVariant(value, valueType, jValEl))
					{
						ET_ASSERT(false,
							"Failed to read value from associative container (type: %s) at index [" ET_FMT_SIZET "]",
							view.get_type().get_name().data(),
							idx);
						return false;
					}

					ET_ASSERT(value.get_type() == valueType);

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
		}
	}

	return true;
}

//------------------------------
// JsonDeserializer::ReadObject
//
bool JsonDeserializer::ReadObject(rttr::variant& var, TypeInfo const& ti, JSON::Value const* const jVal)
{
	ET_ASSERT(jVal->GetType() == JSON::JSON_Object);
	JSON::Object const* const jObj = jVal->obj();

	if (!(jObj->value.empty())) // no need to create an instance if there are no properties to read
	{
		return ReadObjectProperties(rttr::instance(var), ti, jObj);
	}

	return true;
}

//----------------------------------------
// JsonDeserializer::ReadObjectProperties
//
bool JsonDeserializer::ReadObjectProperties(rttr::instance& inst, TypeInfo const& ti, JSON::Object const* const jObj)
{
	for (JSON::Pair const& el : jObj->value)
	{
		// get property from ID
		HashString const propId(el.first.c_str());
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
		if (!ReadVariant(propVar, propType, el.second) || !propVar.is_valid())
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

	return true;
}


} // namespace core
} // namespace et
