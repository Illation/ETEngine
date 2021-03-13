#include "stdafx.h"
#include "JsonSerializer.h"

#include <rttr/type>

#include <EtCore/IO/JsonDom.h>
#include <EtCore/IO/JsonWriter.h>
#include <EtCore/Hashing/HashString.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/AssetRegistration.h>

#include "TypeInfoRegistry.h"
#include "registerMath.h"


namespace et {
namespace core {


//=================
// JSON Serializer
//=================


//-------------------------------
// JsonSerializer::SerializeRoot
//
// Entry point for serialization
//
bool JsonSerializer::SerializeRoot(rttr::variant const& var, JSON::Value*& outVal)
{
	if (TypeInfo::IsBasic(var.get_type())) // for basic types we wrap with typename to make the document type clearer
	{
		JSON::Object* const root = new JSON::Object();
		root->value.emplace_back(var.get_type().get_name().to_string(), nullptr);
		if (!WriteVariant(var, root->value[0].second))
		{
			ET_ASSERT(false, "Failed to write root json content");
			delete root;
			return false;
		}

		outVal = root;
		return true;
	}
	else
	{
		return WriteVariant(var, outVal);
	}
}

//------------------------------
// JsonSerializer::WriteVariant
//
bool JsonSerializer::WriteVariant(rttr::variant const& var, JSON::Value*& outVal)
{
	rttr::type const valueType = var.get_type();
	if (valueType.is_wrapper())
	{
		return WriteVariant(var.extract_wrapped_value(), outVal);
	}

	// for pointers, write either null or the derived type ID
	if (valueType.is_pointer())
	{
		void* val = var.get_value<void*>();
		if (val == nullptr)
		{
			outVal = new JSON::Value();
			return true;
		}

		rttr::instance const inst(var);
		TypeInfo const& ti = TypeInfoRegistry::Instance().GetTypeInfo(inst.get_derived_type());

		// pointers are wrapped into another object layour to allow polymorphism
		outVal = new JSON::Object();
		outVal->obj()->value.emplace_back(ti.m_Type.get_name().to_string(), nullptr);

		// specialization of "WriteBasicVariant" because we already have an instance
		switch (ti.m_Kind)
		{
		case TypeInfo::E_Kind::Arithmetic:
			return WriteArithmeticType(ti.m_Type, var, outVal->obj()->value[0].second);

		case TypeInfo::E_Kind::Enumeration:
			return WriteEnum(var, outVal->obj()->value[0].second);

		case TypeInfo::E_Kind::Vector:
			return WriteVectorType(ti.m_Type, var, outVal->obj()->value[0].second);

		case TypeInfo::E_Kind::String:
			WriteString(var.to_string(), outVal->obj()->value[0].second);
			return true;

		case TypeInfo::E_Kind::Hash:
			WriteHash(var.get_value<HashString>(), outVal->obj()->value[0].second);
			return true;

		case TypeInfo::E_Kind::AssetPointer:
			WriteAssetPtr(var.get_value<I_AssetPtr>(), outVal->obj()->value[0].second);
			return true;

		case TypeInfo::E_Kind::ContainerSequential:
			return WriteSequentialContainer(var.create_sequential_view(), outVal->obj()->value[0].second);

		case TypeInfo::E_Kind::ContainerAssociative:
			return WriteAssociativeContainer(var.create_associative_view(), outVal->obj()->value[0].second);

		case TypeInfo::E_Kind::Class:
			return WriteObject(inst, ti, outVal->obj()->value[0].second);
		}

		ET_ASSERT(false, "unhandled variant type '%s'", ti.m_Id.ToStringDbg());
		return false;
	}
	else
	{
		TypeInfo const& ti = TypeInfoRegistry::Instance().GetTypeInfo(valueType);
		return WriteBasicVariant(var, ti, outVal);
	}
}

//-----------------------------------
// JsonSerializer::WriteBasicVariant
//
bool JsonSerializer::WriteBasicVariant(rttr::variant const& var, TypeInfo const& ti, JSON::Value*& outVal)
{
	// handle different variant kinds
	switch (ti.m_Kind)
	{
	case TypeInfo::E_Kind::Arithmetic:
		return WriteArithmeticType(ti.m_Type, var, outVal);

	case TypeInfo::E_Kind::Enumeration:
		return WriteEnum(var, outVal);

	case TypeInfo::E_Kind::Vector:
		return WriteVectorType(ti.m_Type, var, outVal);

	case TypeInfo::E_Kind::String:
		WriteString(var.to_string(), outVal);
		return true;

	case TypeInfo::E_Kind::Hash:
		WriteHash(var.get_value<HashString>(), outVal);
		return true;

	case TypeInfo::E_Kind::AssetPointer:
		WriteAssetPtr(var.get_value<I_AssetPtr>(), outVal);
		return true;

	case TypeInfo::E_Kind::ContainerSequential:
		return WriteSequentialContainer(var.create_sequential_view(), outVal);

	case TypeInfo::E_Kind::ContainerAssociative:
		return WriteAssociativeContainer(var.create_associative_view(), outVal);

	case TypeInfo::E_Kind::Class:
		return WriteObject(rttr::instance(var), ti, outVal);
	}

	ET_ASSERT(false, "unhandled variant type '%s'", ti.m_Id.ToStringDbg());
	return false;
}

//-------------------------------------
// JsonSerializer::WriteArithmeticType
//
bool JsonSerializer::WriteArithmeticType(rttr::type const type, rttr::variant const& var, JSON::Value*& outVal)
{
	// try bool
	if (type == rttr::type::get<bool>())
	{
		JSON::Bool* jBool = new JSON::Bool();
		jBool->value = var.to_bool();
		outVal = jBool;
		return true;
	}

	// deffo a number
	JSON::Number* jNum = new JSON::Number();
	outVal = jNum;

	// try floating point
	if (type == rttr::type::get<float>())
	{
		jNum->value = var.to_double();
		return true;
	}
	else if (type == rttr::type::get<double>())
	{
		jNum->value = var.to_double();
		return true;
	}

	//deffo an integer type
	jNum->isInt = true;

	if (type == rttr::type::get<char>())
	{
		jNum->valueInt = static_cast<int64>(var.to_uint8());
	}
	else if (type == rttr::type::get<int8>())
	{
		jNum->valueInt = static_cast<int64>(var.to_int8());
	}
	else if (type == rttr::type::get<int16>())
	{
		jNum->valueInt = static_cast<int64>(var.to_int16());
	}
	else if (type == rttr::type::get<int32>())
	{
		jNum->valueInt = static_cast<int64>(var.to_int32());
	}
	else if (type == rttr::type::get<int64>())
	{	
		jNum->valueInt = static_cast<int64>(var.to_int64());
	}
	else if (type == rttr::type::get<uint8>())
	{
		jNum->valueInt = static_cast<int64>(var.to_uint8());
	}
	else if (type == rttr::type::get<uint16>())
	{
		jNum->valueInt = static_cast<int64>(var.to_uint16());
	}
	else if (type == rttr::type::get<uint32>())
	{
		jNum->valueInt = static_cast<int64>(var.to_uint32());
	}
	else if (type == rttr::type::get<uint64>())
	{
		jNum->valueInt = static_cast<int64>(var.to_uint64());
	}
	else // unless it's not handled properly
	{
		delete jNum;

		ET_ASSERT(false, "unhandled arithmetic type '%s'", type.get_name().data());
		outVal = new JSON::Value(); // return true because it's definitly an arithmetic type but make it null
		return false;
	}

	return true;
}

//---------------------------------
// JsonSerializer::WriteVectorType
//
bool JsonSerializer::WriteVectorType(rttr::type const type, rttr::variant const& var, JSON::Value*& outVal)
{
	rttr::array_range<rttr::property> const prop_list = type.get_properties();
	ET_ASSERT(prop_list.size() == 1);

	rttr::property const prop = *prop_list.begin();
	ET_ASSERT(prop.get_name().to_string() == "data");

	rttr::instance const inst(var);
	rttr::type const propValType = prop.get_type();
	rttr::variant const propVar = prop.get_value(inst);

	ET_ASSERT(propValType.is_sequential_container());
	return WriteSequentialContainer(propVar.create_sequential_view(), outVal);
}

//---------------------------
// JsonSerializer::WriteEnum
//
bool JsonSerializer::WriteEnum(rttr::variant const& var, JSON::Value*& outVal)
{		
	bool conversionSuccess = false;

	// first try converting it to a string
	std::string stringVal = var.to_string(&conversionSuccess);
	if (conversionSuccess)
	{
		JSON::String* jString = new JSON::String();
		jString->value = stringVal;
		outVal = jString;
		return true;
	}

	// if that doesn't work we convert it to a uint64 instead
	int64 intVal = var.to_int64(&conversionSuccess);
	if (conversionSuccess)
	{
		JSON::Number* jNum = new JSON::Number();
		jNum->valueInt = intVal;
		jNum->isInt = true;
		outVal = jNum;
		return false;
	}

	// That failed too
	ET_ASSERT(false, "failed to convert enum to string or int, type: %s", var.get_type().get_name().to_string().c_str());
	outVal = new JSON::Value();
	return false;
}

//-------------------------------
// JsonSerializer::WriteAssetPtr
//
void JsonSerializer::WriteAssetPtr(I_AssetPtr const& ptr, JSON::Value*& outVal)
{
	JSON::String* jString = new JSON::String();
	if (!ptr.is_null())
	{
		jString->value = ptr.GetAsset()->GetName();
	}

	outVal = jString;
}

//-----------------------------
// JsonSerializer::WriteString
//
void JsonSerializer::WriteString(std::string const& str, JSON::Value*& outVal)
{
	JSON::String* jString = new JSON::String();
	jString->value = str;
	outVal = jString;
}

//---------------------------
// JsonSerializer::WriteHash
//
void JsonSerializer::WriteHash(HashString const hash, JSON::Value*& outVal)
{
#if ET_HASH_STRING_ENABLED
	char const* const str = hash.GetStoredString();
	if (m_IsVerbose && str != nullptr)
	{
		ET_ASSERT(hash.Get() == GetHash(str));

		JSON::String* jString = new JSON::String();
		jString->value = str;
		outVal = jString;
	}
	else
	{
#endif
		JSON::Number* jNum = new JSON::Number();
		jNum->valueInt = static_cast<int64>(hash.Get());
		jNum->isInt = true;
		outVal = jNum;
#if ET_HASH_STRING_ENABLED
	}
#endif
}

//------------------------------------------
// JsonSerializer::WriteSequentialContainer
//
bool JsonSerializer::WriteSequentialContainer(rttr::variant_sequential_view const& view, JSON::Value*& outVal)
{
	JSON::Array* outArr = new JSON::Array();
	outArr->value.reserve(view.get_size());

	rttr::type const itemType = view.get_value_type();
	if (TypeInfo::IsBasic(itemType))
	{
		TypeInfo const& ti = TypeInfoRegistry::Instance().GetTypeInfo(itemType);

		for (size_t idx = 0u; idx < view.get_size(); ++idx)
		{
			JSON::Value* jItem = nullptr;
			if (!WriteBasicVariant(view.get_value(idx).extract_wrapped_value(), ti, jItem))
			{
				ET_ASSERT(false,
					"Failed to write basic element of sequential container type '%s' at index " ET_FMT_SIZET,
					view.get_type().get_name().data(),
					idx);
				delete outArr;
				return false;
			}

			outArr->value.push_back(jItem);
		}
	}
	else
	{
		for (size_t idx = 0u; idx < view.get_size(); ++idx)
		{
			JSON::Value* jItem = nullptr;
			if (!WriteVariant(view.get_value(idx).extract_wrapped_value(), jItem))
			{
				ET_ASSERT(false,
					"Failed to write element of sequential container type '%s' at index " ET_FMT_SIZET,
					view.get_type().get_name().data(),
					idx);
				delete outArr;
				return false;
			}

			outArr->value.push_back(jItem);
		}
	}

	outVal = outArr;
	return true;
}

//-------------------------------------------
// JsonSerializer::WriteAssociativeContainer
//
bool JsonSerializer::WriteAssociativeContainer(rttr::variant_associative_view const& view, JSON::Value*& outVal)
{
	static const std::string keyName("key");
	static const std::string valueName("value");

	JSON::Array* outArr = new JSON::Array();

	rttr::type const keyType = view.get_key_type();
	if (TypeInfo::IsBasic(keyType))
	{
		TypeInfo const& keyTi = TypeInfoRegistry::Instance().GetTypeInfo(keyType);

		if (view.is_key_only_type())
		{
			for (auto& item : view)
			{
				JSON::Value* jItem = nullptr;
				if (!WriteBasicVariant(item.first, keyTi, jItem))
				{
					ET_ASSERT(false, "Failed to write element of key only associative container type '%s'", view.get_type().get_name().data());
					delete outArr;
					return false;
				}

				outArr->value.push_back(jItem);
			}
		}
		else
		{
			rttr::type const valueType = view.get_value_type();
			if (TypeInfo::IsBasic(valueType))
			{
				TypeInfo const& valueTi = TypeInfoRegistry::Instance().GetTypeInfo(valueType);

				for (auto& item : view)
				{
					JSON::Object* jObj = new JSON::Object();
					jObj->value.emplace_back(keyName, nullptr);
					jObj->value.emplace_back(valueName, nullptr);

					if (!WriteBasicVariant(item.first, keyTi, jObj->value[0].second))
					{
						ET_ASSERT(false, "Failed to write key of associative container type '%s'", view.get_type().get_name().data());
						delete jObj;
						delete outArr;
						return false;
					}

					if (!WriteBasicVariant(item.second, valueTi, jObj->value[1].second))
					{
						ET_ASSERT(false, "Failed to write basic value of associative container type '%s'", view.get_type().get_name().data());
						delete jObj;
						delete outArr;
						return false;
					}

					outArr->value.emplace_back(jObj);
				}
			}
			else
			{
				for (auto& item : view)
				{
					JSON::Object* jObj = new JSON::Object();
					jObj->value.emplace_back(keyName, nullptr);
					jObj->value.emplace_back(valueName, nullptr);

					if (!WriteBasicVariant(item.first, keyTi, jObj->value[0].second))
					{
						ET_ASSERT(false, "Failed to write key of associative container type '%s'", view.get_type().get_name().data());
						delete jObj;
						delete outArr;
						return false;
					}

					if (!WriteVariant(item.second, jObj->value[1].second))
					{
						ET_ASSERT(false, "Failed to write value of associative container type '%s'", view.get_type().get_name().data());
						delete jObj;
						delete outArr;
						return false;
					}

					outArr->value.emplace_back(jObj);
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
				JSON::Value* jItem = nullptr;
				if (!WriteVariant(item.first, jItem))
				{
					ET_ASSERT(false, "Failed to write element of key only associative container type '%s'", view.get_type().get_name().data());
					delete outArr;
					return false;
				}

				outArr->value.push_back(jItem);
			}
		}
		else
		{
			rttr::type const valueType = view.get_value_type();
			if (TypeInfo::IsBasic(valueType))
			{
				TypeInfo const& valueTi = TypeInfoRegistry::Instance().GetTypeInfo(valueType);

				for (auto& item : view)
				{
					JSON::Object* jObj = new JSON::Object();
					jObj->value.emplace_back(keyName, nullptr);
					jObj->value.emplace_back(valueName, nullptr);

					if (!WriteVariant(item.first, jObj->value[0].second))
					{
						ET_ASSERT(false, "Failed to write key of associative container type '%s'", view.get_type().get_name().data());
						delete jObj;
						delete outArr;
						return false;
					}

					if (!WriteBasicVariant(item.second, valueTi, jObj->value[1].second))
					{
						ET_ASSERT(false, "Failed to write basic value of associative container type '%s'", view.get_type().get_name().data());
						delete jObj;
						delete outArr;
						return false;
					}

					outArr->value.emplace_back(jObj);
				}
			}
			else
			{
				for (auto& item : view)
				{
					JSON::Object* jObj = new JSON::Object();
					jObj->value.emplace_back(keyName, nullptr);
					jObj->value.emplace_back(valueName, nullptr);

					if (!WriteVariant(item.first, jObj->value[0].second))
					{
						ET_ASSERT(false, "Failed to write key of associative container type '%s'", view.get_type().get_name().data());
						delete jObj;
						delete outArr;
						return false;
					}

					if (!WriteVariant(item.second, jObj->value[1].second))
					{
						ET_ASSERT(false, "Failed to write value of associative container type '%s'", view.get_type().get_name().data());
						delete jObj;
						delete outArr;
						return false;
					}

					outArr->value.emplace_back(jObj);
				}
			}
		}
	}

	outVal = outArr;
	return true;
}

//-----------------------------
// JsonSerializer::WriteObject
//
bool JsonSerializer::WriteObject(rttr::instance const& inst, TypeInfo const& ti, JSON::Value*& outVal)
{
	JSON::Object* const jObj = new JSON::Object();

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
			delete jObj;
			return false;
		}

		JSON::Pair keyVal = std::make_pair(prop.m_Property.get_name().to_string(), nullptr);
		if (!WriteVariant(propVal, keyVal.second))
		{
			ET_ASSERT(false, "failed to write property '%s' in type '%s'", prop.m_Id.ToStringDbg(), ti.m_Id.ToStringDbg());
			delete jObj;
			return false;
		}

		jObj->value.push_back(keyVal);
	}

	outVal = jObj;
	return true;
}


} // namespace core
} // namespace et
