#include "stdafx.h"
#include "JsonSerializer.h"

#include <rttr/type>

#include <EtCore/IO/JsonDom.h>
#include <EtCore/IO/JsonWriter.h>
#include <EtCore/Hashing/HashString.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/AssetRegistration.h>

#include "registerMath.h"


namespace et {
namespace core {


//=================
// JSON Serializer
//=================


//---------------------------------
// JsonSerializer::ToJsonRecursive
//
// Recursively convert an rttr::instance to a json object - returns false if any properties fail to serialize
//
bool JsonSerializer::ToJsonRecursive(const rttr::instance& inst, JSON::Value*& outJVal, rttr::type const& callingType)
{
	JSON::Object* outJObject = new JSON::Object();
	outJVal = outJObject;

	JSON::Object* appendJObject = outJObject;

	// pointers are wrapped into another object layour to allow polymorphism
	if (callingType.is_pointer())
	{
		rttr::type internalPointerType = inst.get_derived_type().get_raw_type();

		JSON::Pair keyVal = std::make_pair(internalPointerType.get_name().to_string(), new JSON::Object());

		outJObject->value.emplace_back(keyVal);

		appendJObject = keyVal.second->obj();
	}

	rttr::instance instObj = inst.get_type().get_raw_type().is_wrapper() ? inst.get_wrapped_instance() : inst;

	rttr::array_range<rttr::property> const propList = inst.get_derived_type().get_properties();

	bool allPropertiesSerialized = true;

	for (rttr::property const& prop : propList)
	{
		if (prop.get_metadata("NO_SERIALIZE")) // read only should not be serialized probably
		{
			continue;
		}

		rttr::variant const& propVal = prop.get_value(inst);
		if (!propVal)
		{
			continue; // cannot serialize, because we cannot retrieve the value - maybe handle nullptr here
		}

		JSON::Pair keyVal = std::make_pair(prop.get_name().to_string(), nullptr);

		if (!VariantToJsonValue(propVal, keyVal.second))
		{
			LOG("ToJsonRecursive > Failed to serialize property '" + keyVal.first + std::string("' !"), LogLevel::Warning);
			allPropertiesSerialized = false;
		}
		else
		{
			appendJObject->value.push_back(keyVal);
		}
	}

	return allPropertiesSerialized;
}

//---------------------------------
// JsonSerializer::VariantToJsonValue
//
// Recursively convert an rttr::variant to a json value, figuring out it's type in the process
//
bool JsonSerializer::VariantToJsonValue(rttr::variant const& var, JSON::Value*& outVal)
{
	rttr::type valueType = var.get_type();
	rttr::type wrappedType = valueType.is_wrapper() ? valueType.get_wrapped_type() : valueType;

	bool isWrapper = wrappedType != valueType;

	if (valueType.is_pointer())
	{
		void* val = (isWrapper ? var.extract_wrapped_value() : var).get_value<void*>();
		if (val == nullptr)
		{
			outVal = new JSON::Value();
			return true;
		}
	}

	if (AtomicTypeToJsonValue(wrappedType, isWrapper ? var.extract_wrapped_value() : var, outVal))
	{
		// check here if this was a nullptr, if it wasn't and the JSON object is of type JSON_Null we should return false
	}
	else if (var.is_sequential_container())
	{
		if (!ArrayToJsonArray(var.create_sequential_view(), outVal))
		{
			LOG("VariantToJsonValue > Failed to convert variant to json array, typeName: '" + wrappedType.get_name().to_string()
				+ std::string("'!"), LogLevel::Warning);

			return false;
		}
	}
	else if (var.is_associative_container())
	{
		if (!AssociativeContainerToJsonArray(var.create_associative_view(), outVal))
		{
			LOG("VariantToJsonValue > Failed to convert variant to associate view, typeName: '" + wrappedType.get_name().to_string() 
				+ std::string("'!"), LogLevel::Warning);

			return false;
		}
	}
	else
	{
		if (!ToJsonRecursive(var, outVal, wrappedType))
		{
			LOG("VariantToJsonValue > Failed to convert variant to JSON object, typeName: '" + wrappedType.get_name().to_string()
				+ std::string("'!"), LogLevel::Warning);

			return false;
		}
		//if (!(wrappedType.get_properties().empty())) // try converting the variant to a JSON object
		//{
		//}
		//else // if that fails, try converting it to a string
		//{
		//	bool stringConversionSuccess = false;
		//	std::string text = var.to_string(&stringConversionSuccess);

		//	if (!stringConversionSuccess)
		//	{
		//		LOG("VariantToJsonValue > Failed to convert variant to JSON string, result: '" + text + std::string("' typeName: '")
		//			+ wrappedType.get_name().to_string() + std::string("'!"), LogLevel::Warning);

		//		return false;
		//	}

		//	outVal = new JSON::String();
		//	outVal->str()->value = text;
		//}
	}

	return true;
}

//---------------------------------
// JsonSerializer::AtomicTypeToJsonValue
//
// Convert a handful of basic supported "leaf" types to Atomic JSON types. Might have to add more in the future. 
// If value type is not atomic it returns false. If value type is atomic but not supported it returns true and outVal will be of type JSON_Null
//
bool JsonSerializer::AtomicTypeToJsonValue(rttr::type const& valueType, rttr::variant const& var, JSON::Value*& outVal)
{
	if (valueType.is_arithmetic()) // basic arithmetic types get converted to numbers or bools
	{
		if (valueType == rttr::type::get<bool>())
		{
			JSON::Bool* jBool = new JSON::Bool();
			jBool->value = var.to_bool();
			outVal = jBool;
		}
		else if (valueType == rttr::type::get<char>())
		{
			// store char as number because otherwise it would get picked up as a string
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint8());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<int8>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_int8());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<int16>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_int16());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<int32>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_int32());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<int64>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = var.to_int64();
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<uint8>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint8());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<uint16>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint16());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<uint32>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint32());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<uint64>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint64());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<float>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->value = var.to_double();
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<double>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->value = var.to_double();
			outVal = jNum;
		}
		else
		{
			LOG("AtomicTypeToJsonValue > Atomic type not supported, typeName: '" + valueType.get_name().to_string() + std::string("'!")
				, LogLevel::Warning);

			outVal = new JSON::Value(); // return true because it's definitly an arithmetic type but make it null
		}

		return true;
	}
	else if (valueType.is_enumeration()) // enumeration to JSON
	{
		// first try converting it to a string
		bool conversionSuccess = false;
		std::string stringVal = var.to_string(&conversionSuccess);

		if (conversionSuccess)
		{
			JSON::String* jString = new JSON::String();
			jString->value = stringVal;
			outVal = jString;
		}
		else
		{
			// if that doesn't work we convert it to a uint64 instead
			uint64 intVal = var.to_uint64(&conversionSuccess);

			if (conversionSuccess)
			{
				JSON::Number* jNum = new JSON::Number();
				jNum->valueInt = static_cast<int64>(intVal);
				jNum->isInt = true;
				outVal = jNum;
			}
			else
			{
				// That failed too
				// return true because it's definitly an arithmetic type but make it null

				LOG("AtomicTypeToJsonValue > Enum failed to convert, typeName: '" + valueType.get_name().to_string() + std::string("'!")
					, LogLevel::Warning);

				outVal = new JSON::Value();
			}
		}

		return true;
	}
	else if (valueType == rttr::type::get<std::string>()) // string to json
	{
		JSON::String* jString = new JSON::String();
		jString->value = var.to_string();
		outVal = jString;
		return true;
	}
	else if (valueType == rttr::type::get<HashString>()) // hash string to json - depending on the build we store strings or integers
	{
		ET_ASSERT(var.is_type<HashString>());
		HashString const& hash = var.get_value<HashString>();

#if ET_HASH_STRING_ENABLED
		char const* const str = hash.GetStoredString();
		if (str != nullptr)
		{
			ET_ASSERT(hash.Get() == GetHash(str));

			JSON::String* jString = new JSON::String();
			jString->value = str;
			outVal = jString;
			return true;
		}
		else
		{
#endif
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = hash.Get();
			jNum->isInt = true;
			outVal = jNum;
			return true;
#if ET_HASH_STRING_ENABLED
		}
#endif
	}
	else if (IsVectorType(valueType))
	{
		rttr::array_range<rttr::property> const prop_list = valueType.get_properties();
		auto prop = *prop_list.begin();
		ET_ASSERT(prop_list.size() == 1);
		ET_ASSERT(prop.get_name().to_string() == "data");

		rttr::instance inst(var);
		rttr::type propValType = prop.get_type();
		rttr::variant propVar = prop.get_value(inst);

		ET_ASSERT(propValType.is_sequential_container());

		if (ArrayToJsonArray(propVar.create_sequential_view(), outVal))
		{
			return true;
		}
		else
		{
			LOG("VariantToJsonValue > Failed to convert vector type to json array, typeName: '" + valueType.get_name().to_string()
				+ std::string("'!"), LogLevel::Warning);
		}
	}
	else if (IsSerializableAssetPointerType(valueType))
	{
		I_AssetPtr const ptr = var.get_value<I_AssetPtr>();
		JSON::String* jString = new JSON::String();

		if (!ptr.is_null())
		{
			jString->value = ptr.GetAsset()->GetName();
		}

		outVal = jString;
		return true;
	}

	// couldn't handle type, return false to indicate that this should be an object or array
	return false;
}

//---------------------------------
// JsonSerializer::ArrayToJsonArray
//
// Converts a sequential view to a JSON array
//
bool JsonSerializer::ArrayToJsonArray(const rttr::variant_sequential_view& view, JSON::Value*& outVal)
{
	JSON::Array* outArr = new JSON::Array();
	outVal = outArr;

	bool allItemsSucceeded = true;
	
	for (const auto& item : view)
	{
		JSON::Value* jItem = nullptr;

		if (item.is_sequential_container()) // inner array
		{
			if (!ArrayToJsonArray(item.create_sequential_view(), jItem))
			{
				LOG("ArrayToJsonArray > failed to convert array element to inner json array, typeName: '" + 
					item.get_type().get_name().to_string() + std::string("'!"), LogLevel::Warning);

				allItemsSucceeded = false;
			}
		}
		else // atomic type or object
		{
			rttr::variant wrappedVar = item.extract_wrapped_value();
			rttr::type valueType = wrappedVar.get_type();
			if (valueType.is_pointer())
			{
				void* val = wrappedVar.get_value<void*>();
				if (val == nullptr)
				{
					jItem = new JSON::Value();
					outArr->value.emplace_back(jItem);
					continue;
				}
			}

			if (valueType.is_arithmetic() 
				|| (valueType == rttr::type::get<std::string>())
				|| valueType.is_enumeration() 
				|| (valueType == rttr::type::get<HashString>())
				|| IsVectorType(valueType)
				|| IsSerializableAssetPointerType(valueType))
			{
				if (!AtomicTypeToJsonValue(valueType, wrappedVar, jItem))
				{
					LOG("ArrayToJsonArray > failed to convert array element to atomic type, typeName: '" + valueType.get_name().to_string() 
						+ std::string("'!"), LogLevel::Warning);

					allItemsSucceeded = false;
				}
			}
			else // object
			{
				if (!ToJsonRecursive(wrappedVar, jItem, valueType))
				{
					LOG("ArrayToJsonArray > failed to convert array element to json object, typeName: '" + valueType.get_name().to_string()
						+ std::string("'!"), LogLevel::Warning);

					allItemsSucceeded = false;
				}
			}
		}

		outArr->value.emplace_back(jItem);
	}

	return allItemsSucceeded;
}

//---------------------------------
// JsonSerializer::AssociativeContainerToJsonArray
//
// Converts a associative view to a JSON array of objects containing keys and values. If any item fails to be serialized this returns false
//
bool JsonSerializer::AssociativeContainerToJsonArray(const rttr::variant_associative_view& view, JSON::Value*& outVal)
{
	static const std::string keyName("key");
	static const std::string valueName("value");

	JSON::Array* outArr = new JSON::Array();
	outVal = outArr;

	bool allItemsSucceeded = true;

	if (view.is_key_only_type()) // treat this as a normal array
	{
		for (auto& item : view)
		{
			JSON::Value* jItem = nullptr;

			if (!VariantToJsonValue(item.first, jItem))
			{
				LOG("ArrayToJsonArray > failed to convert key only variant to json value!", LogLevel::Warning);
				allItemsSucceeded = false;
			}

			outArr->value.emplace_back(jItem);
		}
	}
	else // treat this as a objects with keys and values
	{
		for (auto& item : view)
		{
			JSON::Object* jObj = new JSON::Object();
			
			// create the key element and place it in the object
			JSON::Pair keyPair = std::make_pair(keyName, nullptr);
			if (!VariantToJsonValue(item.first, keyPair.second))
			{
				LOG("ArrayToJsonArray > failed to convert key variant to json value!", LogLevel::Warning);
				allItemsSucceeded = false;
			}
			jObj->value.emplace_back(keyPair);

			// create the value element and place it in the object
			JSON::Pair valuePair = std::make_pair(valueName, nullptr);
			if (!VariantToJsonValue(item.second, valuePair.second))
			{
				LOG("ArrayToJsonArray > failed to convert value variant to json value!", LogLevel::Warning);
				allItemsSucceeded = false;
			}
			jObj->value.emplace_back(valuePair);

			// place the key value object in the array
			outArr->value.emplace_back(jObj);
		}
	}

	return allItemsSucceeded;
}


} // namespace core
} // namespace et
